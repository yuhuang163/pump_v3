"""Async BLE worker thread using Bleak.

Communicates with the GUI via two thread-safe queues:
  cmd_q  : GUI -> Worker  (commands)
  evt_q  : Worker -> GUI  (events)
"""

from __future__ import annotations

import asyncio
import logging
import queue
import sys
import threading
import time
from dataclasses import dataclass, field
from enum import Enum, auto
from typing import Any, Dict, List, Optional

from bleak import BleakClient, BleakScanner
from bleak.backends.device import BLEDevice
from bleak.backends.scanner import AdvertisementData

from comm_protocol import FrameAssembler, ParsedFrame
from factory_uuids import (
    FACTORY_RX_CHAR_UUID_N,
    FACTORY_SERVICE_UUID,
    FACTORY_TX_CHAR_UUID,
    FACTORY_TX_CHAR_UUID_N,
    REMOTE_RX_CHAR_UUID_N,
    REMOTE_SERVICE_UUID,
    REMOTE_TX_CHAR_UUID,
    REMOTE_TX_CHAR_UUID_N,
    normalize_uuid,
    uuid_in,
)
from ota_img_res_client import ImgResOtaClient, load_ota_file

LOG = logging.getLogger("ble_worker")


class CmdType(Enum):
    SCAN_START = auto()
    SCAN_STOP = auto()
    CONNECT = auto()
    WRITE_RX = auto()
    DISCONNECT = auto()
    OTA_CONNECT = auto()
    OTA_START = auto()
    OTA_STOP = auto()
    SHUTDOWN = auto()


class EvtType(Enum):
    DEVICE_FOUND = auto()
    SCAN_STOPPED = auto()
    CONNECTING = auto()
    FACTORY_ENABLED = auto()
    NOTIFY_DATA = auto()
    WRITE_DONE = auto()
    DISCONNECTED = auto()
    ERROR = auto()
    LOG = auto()
    OTA_READY = auto()
    OTA_PROGRESS = auto()
    OTA_LOG = auto()
    OTA_FINISHED = auto()


@dataclass
class Cmd:
    type: CmdType
    payload: Dict[str, Any] = field(default_factory=dict)


@dataclass
class Evt:
    type: EvtType
    payload: Dict[str, Any] = field(default_factory=dict)


class BleWorker:
    def __init__(self, cmd_q: queue.Queue, evt_q: queue.Queue) -> None:
        self._cmd_q = cmd_q
        self._evt_q = evt_q
        self._thread: Optional[threading.Thread] = None
        self._loop: Optional[asyncio.AbstractEventLoop] = None
        self._client: Optional[BleakClient] = None
        self._scanner: Optional[BleakScanner] = None
        self._scanning = False
        self._connected_addr: Optional[str] = None
        self._connected_name: Optional[str] = None
        self._notify_on = False
        self._notify_uuid: Optional[str] = None
        self._mode: Optional[str] = None  # "factory" | "ota"
        self._expect_local_disconnect = False
        self._stop_flag = threading.Event()

        # OTA RX path
        self._ota_assembler = FrameAssembler()
        self._ota_frame_q: Optional[asyncio.Queue] = None
        self._ota_stop = False
        self._ota_running = False
        self._ota_task: Optional[asyncio.Task] = None
        self._connect_busy = False

    def start(self) -> None:
        if self._thread and self._thread.is_alive():
            return
        self._stop_flag.clear()
        self._thread = threading.Thread(target=self._thread_main, name="BleWorker", daemon=True)
        self._thread.start()

    def stop(self, timeout: float = 5.0) -> None:
        self._cmd_q.put(Cmd(CmdType.SHUTDOWN))
        self._stop_flag.set()
        if self._thread:
            self._thread.join(timeout=timeout)
            self._thread = None

    def _emit(self, evt_type: EvtType, **payload: Any) -> None:
        self._evt_q.put(Evt(evt_type, payload))

    def _log(self, msg: str) -> None:
        LOG.info(msg)
        self._emit(EvtType.LOG, message=msg)

    def _ota_log(self, msg: str) -> None:
        LOG.info("[OTA] %s", msg)
        self._emit(EvtType.OTA_LOG, message=msg)

    def _thread_main(self) -> None:
        self._loop = asyncio.new_event_loop()
        asyncio.set_event_loop(self._loop)
        try:
            self._loop.run_until_complete(self._run())
        finally:
            try:
                pending = asyncio.all_tasks(self._loop)
                for t in pending:
                    t.cancel()
                if pending:
                    self._loop.run_until_complete(asyncio.gather(*pending, return_exceptions=True))
            except Exception:
                pass
            self._loop.close()
            self._loop = None

    async def _run(self) -> None:
        self._log("BLE worker started")
        while not self._stop_flag.is_set():
            try:
                cmd: Cmd = await self._loop.run_in_executor(None, self._cmd_q.get, True, 0.2)
            except queue.Empty:
                continue
            try:
                await self._handle_cmd(cmd)
            except Exception as exc:
                LOG.exception("command failed: %s", cmd.type)
                self._emit(EvtType.ERROR, message=str(exc))
            if cmd.type == CmdType.SHUTDOWN:
                break
        await self._safe_disconnect(silent=True)
        await self._stop_scan()
        self._log("BLE worker stopped")

    async def _handle_cmd(self, cmd: Cmd) -> None:
        if cmd.type == CmdType.SCAN_START:
            await self._start_scan()
        elif cmd.type == CmdType.SCAN_STOP:
            await self._stop_scan()
        elif cmd.type == CmdType.CONNECT:
            cmd = self._coalesce_connect_cmd(cmd)
            if cmd.type == CmdType.OTA_CONNECT:
                await self._connect_ota(
                    cmd.payload.get("address", ""), cmd.payload.get("name")
                )
            else:
                await self._connect_factory(
                    cmd.payload.get("address", ""), cmd.payload.get("name")
                )
        elif cmd.type == CmdType.WRITE_RX:
            await self._write_factory_rx(
                cmd.payload.get("data", b""),
                bool(cmd.payload.get("response", False)),
            )
        elif cmd.type == CmdType.DISCONNECT:
            self._ota_stop = True
            if self._ota_task and not self._ota_task.done():
                try:
                    await asyncio.wait_for(asyncio.shield(self._ota_task), timeout=2.0)
                except Exception:
                    self._ota_task.cancel()
                    try:
                        await self._ota_task
                    except Exception:
                        pass
            await self._safe_disconnect(silent=False)
        elif cmd.type == CmdType.OTA_CONNECT:
            cmd = self._coalesce_connect_cmd(cmd)
            if cmd.type == CmdType.CONNECT:
                await self._connect_factory(
                    cmd.payload.get("address", ""), cmd.payload.get("name")
                )
            else:
                await self._connect_ota(
                    cmd.payload.get("address", ""), cmd.payload.get("name")
                )
        elif cmd.type == CmdType.OTA_START:
            if self._ota_running:
                self._emit(EvtType.ERROR, message="OTA 正在进行中")
                return
            self._ota_task = asyncio.create_task(self._ota_start(cmd.payload))
        elif cmd.type == CmdType.OTA_STOP:
            self._ota_stop = True
            self._ota_log("收到停止请求")
        elif cmd.type == CmdType.SHUTDOWN:
            self._ota_stop = True
            if self._ota_task and not self._ota_task.done():
                try:
                    await asyncio.wait_for(asyncio.shield(self._ota_task), timeout=2.0)
                except Exception:
                    self._ota_task.cancel()
            await self._safe_disconnect(silent=True)
            await self._stop_scan()

    def _on_detect(self, device: BLEDevice, adv: AdvertisementData) -> None:
        name = device.name or adv.local_name or ""
        rssi = adv.rssi if adv.rssi is not None else getattr(device, "rssi", None)
        self._emit(
            EvtType.DEVICE_FOUND,
            address=device.address,
            name=name,
            rssi=rssi,
        )

    async def _start_scan(self) -> None:
        if self._client and self._client.is_connected:
            self._emit(EvtType.ERROR, message="已连接设备时无法扫描，请先退出子页断开")
            return
        if self._scanning:
            return
        await self._stop_scan()
        self._scanner = BleakScanner(detection_callback=self._on_detect)
        await self._scanner.start()
        self._scanning = True
        self._log("scan started")

    async def _stop_scan(self) -> None:
        if self._scanner is not None:
            try:
                await self._scanner.stop()
            except Exception as exc:
                LOG.warning("stop scan: %s", exc)
            self._scanner = None
        if self._scanning:
            self._scanning = False
            self._log("scan stopped")
            self._emit(EvtType.SCAN_STOPPED)

    async def _prepare_connect(self, address: str) -> bool:
        """Stop scan and settle WinRT before connect. False = reject (busy)."""
        if self._connect_busy:
            self._log(
                f"连接进行中，忽略重复请求 → {address}"
            )
            return False
        self._connect_busy = True
        await self._stop_scan()
        if self._client and self._client.is_connected:
            await self._safe_disconnect(silent=True)
        # WinRT often returns E_ABORT if connect follows scan stop too quickly.
        if sys.platform == "win32":
            self._log("waiting 0.6s after stop scan (Windows BLE settle)")
            await asyncio.sleep(0.6)
        return True

    def _finish_connect_attempt(self) -> None:
        self._connect_busy = False

    @staticmethod
    def _format_connect_error(exc: BaseException) -> str:
        text = str(exc).strip()
        winerr = getattr(exc, "winerror", None)
        # E_ABORT = -2147467260 (0x80004004)
        if winerr in (-2147467260, 0x80004004) or "已中止" in text:
            return (
                "连接被中止 (E_ABORT)。常见原因：重复点击连接、停扫后立刻连、"
                "或上一笔连接未结束。请停止扫描，等 1～2 秒后再连接一次。"
            )
        if text:
            if winerr is not None:
                return f"{text} (WinError {winerr})"
            return text
        if winerr is not None:
            return f"{type(exc).__name__} WinError {winerr}"
        return repr(exc)

    def _coalesce_connect_cmd(self, cmd: Cmd) -> Cmd:
        """If several CONNECT/OTA_CONNECT were queued by rapid clicks, keep the last."""
        latest = cmd
        deferred: List[Cmd] = []
        while True:
            try:
                nxt: Cmd = self._cmd_q.get_nowait()
            except queue.Empty:
                break
            if nxt.type in (CmdType.CONNECT, CmdType.OTA_CONNECT):
                self._log(
                    f"coalesce connect: drop {latest.payload.get('address')} "
                    f"→ keep {nxt.payload.get('address')}"
                )
                latest = nxt
            else:
                deferred.append(nxt)
        for d in deferred:
            self._cmd_q.put(d)
        return latest

    async def _discover_services(self, client: BleakClient) -> List[str]:
        try:
            services = client.services
            if services is None:
                await client.get_services()
                services = client.services
        except Exception:
            try:
                await client.get_services()
            except Exception:
                pass
            services = getattr(client, "services", None)

        svc_uuids: List[str] = []
        if services is not None:
            for svc in services:
                raw = str(svc.uuid)
                svc_uuids.append(raw)
                try:
                    self._log(f"  service: {raw} -> {normalize_uuid(raw)}")
                except ValueError:
                    self._log(f"  service: {raw}")
        return svc_uuids

    async def _connect_factory(self, address: str, name: Optional[str]) -> None:
        if not address:
            self._emit(EvtType.ERROR, message="未指定设备地址")
            return
        if not await self._prepare_connect(address):
            return

        self._emit(EvtType.CONNECTING, address=address, name=name or "")
        self._log(f"connecting (Factory) to {address} ...")
        client = BleakClient(address, disconnected_callback=self._on_disconnected)
        try:
            await client.connect(timeout=20.0)
        except Exception as exc:
            self._finish_connect_attempt()
            self._emit(
                EvtType.ERROR,
                message=f"连接失败: {self._format_connect_error(exc)}",
            )
            return

        self._client = client
        self._connected_addr = address
        self._connected_name = name or ""
        self._mode = "factory"

        try:
            await self._enable_factory(client)
        except Exception as exc:
            LOG.exception("enable factory failed")
            await self._safe_disconnect(silent=True)
            self._finish_connect_attempt()
            self._emit(EvtType.ERROR, message=str(exc) or repr(exc))
            return

        self._finish_connect_attempt()
        self._log(f"Factory enabled on {address}")
        self._emit(
            EvtType.FACTORY_ENABLED,
            address=address,
            name=self._connected_name,
        )

    async def _enable_factory(self, client: BleakClient) -> None:
        svc_uuids = await self._discover_services(client)
        try:
            expect_svc = normalize_uuid(FACTORY_SERVICE_UUID)
            tx_uuid = normalize_uuid(FACTORY_TX_CHAR_UUID)
        except ValueError as exc:
            raise RuntimeError(f"UUID 配置非法: {exc}") from exc

        if svc_uuids and not uuid_in(expect_svc, svc_uuids):
            raise RuntimeError(
                "已连接但未找到 Factory Service\n"
                f"期望: {FACTORY_SERVICE_UUID} (= {expect_svc})\n"
                f"设备服务: {', '.join(svc_uuids)}"
            )

        def _on_notify(_sender: int, data: bytearray) -> None:
            self._emit(
                EvtType.NOTIFY_DATA,
                address=self._connected_addr or "",
                data=bytes(data),
                ts=time.time(),
            )

        await client.start_notify(tx_uuid, _on_notify)
        self._notify_on = True
        self._notify_uuid = tx_uuid
        self._log(f"Factory TX notify ON uuid={tx_uuid}")

    async def _connect_ota(self, address: str, name: Optional[str]) -> None:
        if not address:
            self._emit(EvtType.ERROR, message="未指定设备地址")
            return
        if not await self._prepare_connect(address):
            return

        self._emit(EvtType.CONNECTING, address=address, name=name or "")
        self._log(f"connecting (OTA/Remote) to {address} ...")
        client = BleakClient(address, disconnected_callback=self._on_disconnected)
        try:
            await client.connect(timeout=20.0)
        except Exception as exc:
            self._finish_connect_attempt()
            self._emit(
                EvtType.ERROR,
                message=f"连接失败: {self._format_connect_error(exc)}",
            )
            return

        self._client = client
        self._connected_addr = address
        self._connected_name = name or ""
        self._mode = "ota"
        self._ota_assembler.reset()
        self._ota_frame_q = asyncio.Queue()
        self._ota_stop = False

        try:
            await self._enable_remote(client)
            mtu = getattr(client, "mtu_size", None)
            self._log(f"MTU size={mtu}")
        except Exception as exc:
            LOG.exception("enable remote failed")
            await self._safe_disconnect(silent=True)
            self._finish_connect_attempt()
            self._emit(EvtType.ERROR, message=str(exc) or repr(exc))
            return

        self._finish_connect_attempt()
        self._log(f"Remote OTA ready on {address}")
        self._emit(
            EvtType.OTA_READY,
            address=address,
            name=self._connected_name,
        )

    async def _enable_remote(self, client: BleakClient) -> None:
        svc_uuids = await self._discover_services(client)
        try:
            expect_svc = normalize_uuid(REMOTE_SERVICE_UUID)
            tx_uuid = normalize_uuid(REMOTE_TX_CHAR_UUID)
        except ValueError as exc:
            raise RuntimeError(f"UUID 配置非法: {exc}") from exc

        if svc_uuids and not uuid_in(expect_svc, svc_uuids):
            raise RuntimeError(
                "已连接但未找到 Remote Service\n"
                f"期望: {REMOTE_SERVICE_UUID} (= {expect_svc})\n"
                f"设备服务: {', '.join(svc_uuids)}"
            )

        def _on_notify(_sender: int, data: bytearray) -> None:
            raw = bytes(data)
            frames = self._ota_assembler.feed(raw)
            q = self._ota_frame_q
            for fr in frames:
                if q is not None and self._loop is not None:
                    self._loop.call_soon_threadsafe(q.put_nowait, fr)

        await client.start_notify(tx_uuid, _on_notify)
        self._notify_on = True
        self._notify_uuid = tx_uuid
        self._log(f"Remote TX notify ON uuid={tx_uuid}")

    def _on_disconnected(self, _client: BleakClient) -> None:
        was = self._connected_addr
        local = self._expect_local_disconnect
        self._expect_local_disconnect = False
        self._notify_on = False
        self._notify_uuid = None
        self._client = None
        self._connected_addr = None
        self._connected_name = None
        self._mode = None
        self._ota_stop = True
        self._connect_busy = False
        self._log(f"device disconnected: {was} local={local}")
        if not local:
            self._emit(EvtType.DISCONNECTED, reason="remote_or_lost")

    async def _write_factory_rx(self, data: bytes, response: bool) -> None:
        if not self._client or not self._client.is_connected:
            self._emit(EvtType.ERROR, message="未连接，无法发送")
            return
        if not data:
            self._emit(EvtType.ERROR, message="发送内容为空")
            return
        try:
            await self._client.write_gatt_char(
                FACTORY_RX_CHAR_UUID_N, data, response=response
            )
            self._log(f"Factory RX write {len(data)}B response={response}")
            self._emit(EvtType.WRITE_DONE, length=len(data))
        except Exception as exc:
            self._emit(EvtType.ERROR, message=f"写入失败: {exc}")

    async def _write_remote_rx_chunked(self, data: bytes) -> None:
        if not self._client or not self._client.is_connected:
            raise RuntimeError("未连接，无法写入 Remote RX")
        mtu = getattr(self._client, "mtu_size", None) or 23
        # Keep ATT payload within MTU-3. On Windows, Write CMD drops easily if
        # flooded — use smaller chunks + pacing.
        chunk = max(20, int(mtu) - 3)
        if sys.platform == "win32":
            chunk = min(chunk, 100)
            pace_s = 0.006
        else:
            chunk = min(chunk, 180)
            pace_s = 0.001
        for i in range(0, len(data), chunk):
            part = data[i : i + chunk]
            await self._client.write_gatt_char(
                REMOTE_RX_CHAR_UUID_N, part, response=False
            )
            await asyncio.sleep(pace_s)
        # Let the controller flush the last Write CMD before we wait for rsp.
        if sys.platform == "win32":
            await asyncio.sleep(0.02)

    async def _ota_wait_frame(self, timeout: float) -> Optional[ParsedFrame]:
        q = self._ota_frame_q
        if q is None:
            return None
        try:
            return await asyncio.wait_for(q.get(), timeout=timeout)
        except asyncio.TimeoutError:
            return None

    async def _ota_start(self, payload: Dict[str, Any]) -> None:
        if self._mode != "ota" or not self._client or not self._client.is_connected:
            self._emit(EvtType.ERROR, message="未处于 OTA 连接状态")
            return
        if self._ota_running:
            self._emit(EvtType.ERROR, message="OTA 正在进行中")
            return

        path = payload.get("path", "")
        img_id = int(payload.get("img_id", 1))
        version = int(payload.get("version", 0x00010000))

        try:
            meta = load_ota_file(path)
        except Exception as exc:
            self._emit(EvtType.ERROR, message=f"读取文件失败: {exc}")
            self._emit(EvtType.OTA_FINISHED, success=False, message=str(exc))
            return

        self._ota_stop = False
        self._ota_running = True
        self._ota_assembler.reset()
        # Drain stale frames
        q = self._ota_frame_q
        if q is not None:
            while not q.empty():
                try:
                    q.get_nowait()
                except asyncio.QueueEmpty:
                    break

        def _progress(offset: int, total: int, stage: str) -> None:
            self._emit(EvtType.OTA_PROGRESS, offset=offset, total=total, stage=stage)

        client = ImgResOtaClient(
            send_frame=self._write_remote_rx_chunked,
            wait_frame=self._ota_wait_frame,
            should_stop=lambda: self._ota_stop,
            log=self._ota_log,
            progress=_progress,
        )

        success = False
        message = ""
        try:
            success = await client.run(meta, img_id=img_id, version=version)
            message = "成功" if success else "已停止"
        except Exception as exc:
            LOG.exception("OTA failed")
            message = str(exc)
            self._ota_log(f"失败: {message}")
            success = False
        finally:
            self._ota_running = False

        self._emit(EvtType.OTA_FINISHED, success=success, message=message)

    async def _safe_disconnect(self, silent: bool) -> None:
        client = self._client
        if client is None:
            if not silent:
                self._emit(EvtType.DISCONNECTED, reason="already")
            return
        self._expect_local_disconnect = True
        self._ota_stop = True
        try:
            if self._notify_on and client.is_connected and self._notify_uuid:
                try:
                    await client.stop_notify(self._notify_uuid)
                except Exception as exc:
                    LOG.warning("stop_notify: %s", exc)
            self._notify_on = False
            self._notify_uuid = None
            if client.is_connected:
                await client.disconnect()
        except Exception as exc:
            LOG.warning("disconnect: %s", exc)
            self._expect_local_disconnect = False
        finally:
            self._client = None
            self._connected_addr = None
            self._connected_name = None
            self._mode = None
            self._ota_running = False
            if not silent:
                self._emit(EvtType.DISCONNECTED, reason="local")
            self._log("disconnected")
