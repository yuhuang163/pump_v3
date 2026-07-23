"""Image-resource OTA session (START → WRITE → END) over comm_protocol."""

from __future__ import annotations

import asyncio
import logging
import zlib
from dataclasses import dataclass
from typing import Awaitable, Callable, Optional

from comm_protocol import (
    FRAME_TYPE_NOTIFY,
    FRAME_TYPE_RESPONSE,
    MAX_IMG_RES_SIZE,
    OTA_BLOCK_SIZE,
    OTA_STATUS_DATA_BLOCK_TIMEOUT,
    OTA_STATUS_DATA_CRC_ERROR,
    OTA_STATUS_FAIL,
    OTA_STATUS_SUCCESS,
    OTA_TLV_IMG_OFFSET_ADDR,
    OTA_TLV_UPGRADE_RESULT,
    OTA_TLV_UPGRADE_STATUS_NOTIFY,
    PROTOCOL_SERVICE_ERROR_CODE_TLV_ID,
    ParsedFrame,
    build_ota_end_frame,
    build_ota_start_frame,
    build_ota_write_frame,
    get_ota_service,
    get_tlv_u16,
    get_tlv_u32,
)

LOG = logging.getLogger("ota_img_res")

LogFn = Callable[[str], None]
ProgressFn = Callable[[int, int, str], None]  # offset, total, stage
SendFn = Callable[[bytes], Awaitable[None]]
WaitFn = Callable[[float], Awaitable[Optional[ParsedFrame]]]
StopFn = Callable[[], bool]

# WRITE/START Response wait; END CRC on large images can be slower.
DEFAULT_RESPONSE_TIMEOUT_S = 10.0
DEFAULT_END_TIMEOUT_S = 30.0
# F000=0 only means queued; give Flash worker time before END / next block.
DEFAULT_POST_WRITE_DELAY_S = 0.05
DEFAULT_PRE_END_DELAY_S = 0.8

_STATUS_HINTS = {
    OTA_STATUS_DATA_CRC_ERROR: "全量 CRC32 校验失败（Flash 内容与 START 中的 CRC 不一致，常见于 BLE 写丢包）",
    OTA_STATUS_DATA_BLOCK_TIMEOUT: "块接收超时",
    0x0006: "数据大小/空洞错误",
    0x0009: "Flash 写入失败",
    OTA_STATUS_FAIL: "升级失败",
}


@dataclass
class OtaFileMeta:
    path: str
    data: bytes
    size: int
    crc32: int


def load_ota_file(path: str) -> OtaFileMeta:
    with open(path, "rb") as f:
        data = f.read()
    if not data:
        raise ValueError("资源文件为空")
    if len(data) > MAX_IMG_RES_SIZE:
        raise ValueError(f"文件过大: {len(data)} > {MAX_IMG_RES_SIZE} (7MB)")
    crc = zlib.crc32(data) & 0xFFFFFFFF
    return OtaFileMeta(path=path, data=data, size=len(data), crc32=crc)


class ImgResOtaClient:
    """Run one image-resource OTA session."""

    def __init__(
        self,
        *,
        send_frame: SendFn,
        wait_frame: WaitFn,
        should_stop: StopFn,
        log: LogFn,
        progress: ProgressFn,
        response_timeout: float = DEFAULT_RESPONSE_TIMEOUT_S,
        end_timeout: float = DEFAULT_END_TIMEOUT_S,
        post_write_delay: float = DEFAULT_POST_WRITE_DELAY_S,
        pre_end_delay: float = DEFAULT_PRE_END_DELAY_S,
    ) -> None:
        self._send = send_frame
        self._wait = wait_frame
        self._should_stop = should_stop
        self._log = log
        self._progress = progress
        self._timeout = response_timeout
        self._end_timeout = end_timeout
        self._post_write_delay = post_write_delay
        self._pre_end_delay = pre_end_delay
        self._seq = 1

    def _next_seq(self) -> int:
        seq = self._seq & 0xFF
        self._seq = (self._seq + 1) & 0xFF
        if self._seq == 0:
            self._seq = 1
        return seq

    async def _wait_response(
        self, seq: int, timeout: Optional[float] = None
    ) -> ParsedFrame:
        """Wait for RESPONSE(seq) or relevant NOTIFY; raise on fatal notify / timeout."""
        wait_s = self._timeout if timeout is None else timeout
        while True:
            if self._should_stop():
                raise RuntimeError("OTA 已停止")
            frame = await self._wait(wait_s)
            if frame is None:
                raise TimeoutError(f"等待应答超时 (seq={seq}, {wait_s}s)")

            if frame.frame_type == FRAME_TYPE_NOTIFY:
                fatal, retry, code, resume = self._handle_notify(frame)
                if retry:
                    raise _RetryBlock(code, resume_offset=resume)
                if fatal:
                    hint = _STATUS_HINTS.get(code, "")
                    msg = f"设备 Notify 终止升级: code=0x{code:04X}"
                    if hint:
                        msg = f"{msg} — {hint}"
                    raise RuntimeError(msg)
                continue

            if frame.frame_type == FRAME_TYPE_RESPONSE and frame.seq == seq:
                return frame

            self._log(
                f"忽略帧 type={frame.frame_type} seq={frame.seq} (expect rsp seq={seq})"
            )

    def _handle_notify(
        self, frame: ParsedFrame
    ) -> tuple[bool, bool, int, Optional[int]]:
        """Return (fatal, retry, code, resume_offset).

        Firmware Notify carries 0x0008=recv_size + 0x000C=status
        (see ota_tlvs_upgrade_status_notify).
        """
        svc = get_ota_service(frame)
        if svc is None:
            self._log(f"NOTIFY 无 OTA service, raw={len(frame.raw)}B")
            return False, False, 0, None
        code = get_tlv_u16(svc, OTA_TLV_UPGRADE_STATUS_NOTIFY)
        resume = get_tlv_u32(svc, OTA_TLV_IMG_OFFSET_ADDR)
        if code is None:
            self._log("NOTIFY 无 0x000C")
            return False, False, 0, resume
        self._log(
            f"NOTIFY 0x000C code=0x{code:04X}"
            + (f" resume_offset=0x{resume:X}" if resume is not None else "")
        )
        if code == OTA_STATUS_DATA_BLOCK_TIMEOUT:
            self._log("块超时 → 按设备断点重发")
            return False, True, code, resume
        if code == OTA_STATUS_SUCCESS:
            return False, False, code, resume
        return True, False, code, resume

    def _check_f000(self, frame: ParsedFrame, context: str) -> None:
        svc = get_ota_service(frame)
        if svc is None:
            raise RuntimeError(f"{context}: 应答无 OTA Service")
        err = get_tlv_u16(svc, PROTOCOL_SERVICE_ERROR_CODE_TLV_ID)
        if err is None:
            self._log(f"{context}: 无 F000，继续")
            return
        if err != OTA_STATUS_SUCCESS:
            raise RuntimeError(f"{context}: F000=0x{err:04X} 失败")

    def _apply_resume(
        self, current: int, resume: Optional[int], total: int, why: str
    ) -> int:
        """Snap write cursor to device recv_size when Notify provides 0x0008."""
        if resume is None:
            self._log(f"{why}: Notify 无 0x0008，保持本地 offset=0x{current:X}")
            return current
        if resume > total:
            raise RuntimeError(f"{why}: 设备断点 0x{resume:X} > total={total}")
        if resume != current:
            self._log(
                f"{why}: 回退到设备断点 offset=0x{resume:X} (本地曾为 0x{current:X})"
            )
        else:
            self._log(f"{why}: 重试 offset=0x{resume:X}")
        return resume

    async def _start_upgrade(
        self,
        *,
        img_id: int,
        version: int,
        crc32: int,
        total: int,
    ) -> tuple[int, int]:
        """Send START; return (resume_offset, block_size)."""
        seq = self._next_seq()
        start_frame = build_ota_start_frame(seq, img_id, version, crc32, total)
        await self._send(start_frame)
        self._log(
            f"START req seq={seq} img_id={img_id} version=0x{version:08X} "
            f"({len(start_frame)}B)"
        )
        try:
            rsp = await self._wait_response(seq)
        except _RetryBlock as exc:
            raise RuntimeError(
                f"START 阶段收到块超时 Notify: 0x{exc.code:04X}"
            ) from exc
        self._check_f000(rsp, "START")
        svc = get_ota_service(rsp)
        assert svc is not None
        offset = get_tlv_u32(svc, OTA_TLV_IMG_OFFSET_ADDR)
        block = get_tlv_u16(svc, 0x0006)  # BLOCK_SIZE
        if offset is None:
            offset = 0
        if block is None or block == 0:
            block = OTA_BLOCK_SIZE
        self._log(f"START rsp offset={offset} block={block} F000=0")
        if offset > total:
            raise RuntimeError(f"断点 offset={offset} > total={total}")
        return offset, block

    async def _run_once(
        self,
        meta: OtaFileMeta,
        img_id: int,
        version: int,
        *,
        force_full: bool = False,
    ) -> bool:
        data = meta.data
        total = meta.size
        ver = version
        if force_full:
            ver = (ver + 1) & 0xFFFFFFFF
            self._log(
                f"强制全量重写: version 0x{version:08X} → 0x{ver:08X} "
                f"（避免设备沿用失败残留断点）"
            )

        self._progress(0, total, "STARTING")
        self._log(
            f"SESSION img_id={img_id} version=0x{ver:08X} "
            f"size={total} crc32=0x{meta.crc32:08X}"
        )

        offset, block = await self._start_upgrade(
            img_id=img_id, version=ver, crc32=meta.crc32, total=total
        )

        # Device may resume at total after a previous CRC-failed attempt that
        # still persisted recv_size==total. Flash can be 0xFF (CRC 0x1b43eabd).
        if offset >= total and not force_full:
            self._log(
                f"设备断点已到末尾 ({offset}/{total})，疑似上次失败残留；"
                f"bump version 后全量重写"
            )
            return await self._run_once(meta, img_id, ver, force_full=True)

        if offset >= total and force_full:
            raise RuntimeError(
                f"强制重开后断点仍为 {offset}/{total}，无法全量写入"
            )

        self._progress(offset, total, "WRITING")

        # --- WRITE ---
        while offset < total:
            if self._should_stop():
                self._log("用户停止 OTA")
                self._progress(offset, total, "STOPPED")
                return False

            chunk_len = min(block, total - offset)
            chunk = data[offset : offset + chunk_len]
            seq = self._next_seq()
            wframe = build_ota_write_frame(seq, offset, chunk)
            await self._send(wframe)
            self._log(f"WRITE offset=0x{offset:X} len={chunk_len} seq={seq}")
            try:
                rsp = await self._wait_response(seq)
                self._check_f000(rsp, f"WRITE@{offset:#x}")
            except _RetryBlock as exc:
                offset = self._apply_resume(
                    offset, exc.resume_offset, total, "块超时重试"
                )
                self._progress(offset, total, "WRITING")
                continue

            offset += chunk_len
            self._progress(offset, total, "WRITING")
            if self._post_write_delay > 0:
                await asyncio.sleep(self._post_write_delay)

        # --- END ---
        if self._pre_end_delay > 0:
            self._log(
                f"等待 Flash 队列排空 {self._pre_end_delay:.1f}s 后发送 END…"
            )
            await asyncio.sleep(self._pre_end_delay)
        self._progress(total, total, "ENDING")
        seq = self._next_seq()
        eframe = build_ota_end_frame(seq, img_id=img_id)
        await self._send(eframe)
        self._log(f"END req seq={seq} (timeout={self._end_timeout}s)")
        try:
            rsp = await self._wait_response(seq, timeout=self._end_timeout)
        except _RetryBlock as exc:
            raise RuntimeError(
                f"END 阶段块超时 Notify: 0x{exc.code:04X}"
            ) from exc
        self._check_f000(rsp, "END")
        svc = get_ota_service(rsp)
        assert svc is not None
        result = get_tlv_u16(svc, OTA_TLV_UPGRADE_RESULT)
        if result is None:
            raise RuntimeError("END 应答缺少 UPGRADE_RESULT")
        self._log(f"END rsp UPGRADE_RESULT=0x{result:04X}")
        if result != OTA_STATUS_SUCCESS:
            self._progress(total, total, "FAILED")
            hint = _STATUS_HINTS.get(result, "")
            msg = f"升级失败 UPGRADE_RESULT=0x{result:04X}"
            if hint:
                msg = f"{msg} — {hint}"
            if result == OTA_STATUS_DATA_CRC_ERROR:
                raise _CrcFailed(msg)
            raise RuntimeError(msg)

        self._progress(total, total, "SUCCESS")
        self._log("OTA 成功")
        return True

    async def run(
        self,
        meta: OtaFileMeta,
        img_id: int,
        version: int,
    ) -> bool:
        try:
            return await self._run_once(meta, img_id, version, force_full=False)
        except _CrcFailed as exc:
            # Failed CRC often leaves STORE resume at recv_size==total; retry
            # must use a new version base so force_full bumps past the stale one.
            self._log(f"{exc} → 自动 bump version 全量重试一次")
            return await self._run_once(
                meta, img_id, (version + 1) & 0xFFFFFFFF, force_full=True
            )
        except RuntimeError as exc:
            text = str(exc)
            if "0x0007" in text or "CRC32" in text:
                self._log(f"{exc} → 自动 bump version 全量重试一次")
                return await self._run_once(
                    meta, img_id, (version + 1) & 0xFFFFFFFF, force_full=True
                )
            raise


class _CrcFailed(RuntimeError):
    pass


class _RetryBlock(Exception):
    def __init__(self, code: int, resume_offset: Optional[int] = None) -> None:
        self.code = code
        self.resume_offset = resume_offset
        super().__init__(
            f"retry block code=0x{code:04X} resume={resume_offset!r}"
        )
