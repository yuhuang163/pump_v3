"""Tkinter GUI: scan page + Factory session page + OTA page."""

from __future__ import annotations

import logging
import queue
import re
import tkinter as tk
from datetime import datetime
from pathlib import Path
from tkinter import filedialog, messagebox, ttk
from typing import Dict, Optional, Tuple

from ble_worker import BleWorker, Cmd, CmdType, Evt, EvtType
from ota_img_res_client import load_ota_file

LOG = logging.getLogger("gui_app")

APP_TITLE = "BleFactoryTool"
APP_VERSION = "0.6"


def parse_hex_payload(text: str) -> bytes:
    """Parse space/comma/0x separated hex into bytes."""
    cleaned = text.strip()
    if not cleaned:
        raise ValueError("发送内容为空")
    cleaned = cleaned.replace(",", " ").replace("\n", " ").replace("\t", " ")
    cleaned = re.sub(r"0[xX]", " ", cleaned)
    parts = [p for p in cleaned.split() if p]
    if not parts:
        hex_only = re.sub(r"[^0-9a-fA-F]", "", text)
        if len(hex_only) == 0:
            raise ValueError("发送内容为空")
        if len(hex_only) % 2 != 0:
            raise ValueError("Hex 位数为奇数，请成对填写")
        return bytes.fromhex(hex_only)
    out = bytearray()
    for p in parts:
        if not re.fullmatch(r"[0-9a-fA-F]{1,2}", p):
            raise ValueError(f"非法 Hex: {p}")
        out.append(int(p, 16))
    return bytes(out)


def format_hex(data: bytes) -> str:
    return " ".join(f"{b:02X}" for b in data)


def format_utf8(data: bytes) -> str:
    return data.decode("utf-8", errors="replace")


def parse_int_field(text: str, name: str) -> int:
    raw = text.strip()
    if not raw:
        raise ValueError(f"{name} 为空")
    try:
        return int(raw, 0)  # supports 0x… and decimal
    except ValueError as exc:
        raise ValueError(f"{name} 非法: {raw}") from exc


class BleFactoryApp(tk.Tk):
    def __init__(self) -> None:
        super().__init__()
        self.title(f"{APP_TITLE}  v{APP_VERSION}")
        self.geometry("820x700")
        self.minsize(700, 560)

        self._cmd_q: queue.Queue = queue.Queue()
        self._evt_q: queue.Queue = queue.Queue()
        self._worker = BleWorker(self._cmd_q, self._evt_q)
        self._worker.start()

        self._devices: Dict[str, Dict] = {}
        self._selected_addr: Optional[str] = None
        # IDLE / SCANNING / CONNECTING / SESSION / OTA_SESSION / OTA_RUNNING / DISCONNECTING
        self._state = "IDLE"
        self._filter_var = tk.StringVar(value="")
        self._status_var = tk.StringVar(value="状态: 空闲")
        self._selected_var = tk.StringVar(value="选中: (无)")
        self._session_info_var = tk.StringVar(value="")
        self._ota_info_var = tk.StringVar(value="")
        self._send_fmt = tk.StringVar(value="hex")
        self._recv_fmt = tk.StringVar(value="hex")
        self._write_response = tk.BooleanVar(value=False)
        self._recv_paused = tk.BooleanVar(value=False)
        self._paused_count = 0

        self._ota_path_var = tk.StringVar(value="")
        self._ota_img_id_var = tk.StringVar(value="1")
        self._ota_version_var = tk.StringVar(value="0x00010000")
        self._ota_file_info_var = tk.StringVar(value="文件信息: (未选择)")
        self._ota_progress_var = tk.StringVar(value="进度: —")
        self._ota_stage_var = tk.StringVar(value="阶段: IDLE")
        self._ota_file_meta = None

        self._build_ui()
        self._show_scan_page()
        self._poll_events()
        self.protocol("WM_DELETE_WINDOW", self._on_close)

    # ------------------------------------------------------------------ UI
    def _build_ui(self) -> None:
        self._container = ttk.Frame(self)
        self._container.pack(fill=tk.BOTH, expand=True)

        self._scan_frame = ttk.Frame(self._container, padding=8)
        self._session_frame = ttk.Frame(self._container, padding=8)
        self._ota_frame = ttk.Frame(self._container, padding=8)
        self._build_scan_page(self._scan_frame)
        self._build_session_page(self._session_frame)
        self._build_ota_page(self._ota_frame)

    def _build_scan_page(self, parent: ttk.Frame) -> None:
        top = ttk.Frame(parent)
        top.pack(fill=tk.X)

        self._btn_scan = ttk.Button(top, text="开始扫描", command=self._on_scan_start)
        self._btn_scan.pack(side=tk.LEFT, padx=(0, 6))
        self._btn_stop = ttk.Button(top, text="停止扫描", command=self._on_scan_stop, state=tk.DISABLED)
        self._btn_stop.pack(side=tk.LEFT, padx=(0, 12))

        ttk.Label(top, text="过滤:").pack(side=tk.LEFT)
        filt = ttk.Entry(top, textvariable=self._filter_var, width=20)
        filt.pack(side=tk.LEFT, padx=4)
        self._filter_var.trace_add("write", lambda *_: self._refresh_device_list())

        cols = ("name", "address", "rssi")
        tree_frame = ttk.Frame(parent)
        tree_frame.pack(fill=tk.BOTH, expand=True, pady=8)
        self._tree = ttk.Treeview(tree_frame, columns=cols, show="headings", selectmode="browse")
        self._tree.heading("name", text="名称")
        self._tree.heading("address", text="地址")
        self._tree.heading("rssi", text="RSSI")
        self._tree.column("name", width=220, anchor=tk.W)
        self._tree.column("address", width=220, anchor=tk.W)
        self._tree.column("rssi", width=80, anchor=tk.CENTER)
        scroll = ttk.Scrollbar(tree_frame, orient=tk.VERTICAL, command=self._tree.yview)
        self._tree.configure(yscrollcommand=scroll.set)
        self._tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scroll.pack(side=tk.RIGHT, fill=tk.Y)
        self._tree.bind("<<TreeviewSelect>>", self._on_tree_select)
        self._tree.bind("<Double-1>", lambda _e: self._on_connect())

        mid = ttk.Frame(parent)
        mid.pack(fill=tk.X)
        ttk.Label(mid, textvariable=self._selected_var).pack(side=tk.LEFT)
        self._btn_ota = ttk.Button(mid, text="OTA 测试", command=self._on_ota_connect)
        self._btn_ota.pack(side=tk.RIGHT, padx=(6, 0))
        self._btn_connect = ttk.Button(mid, text="连接并使能 Factory", command=self._on_connect)
        self._btn_connect.pack(side=tk.RIGHT)

        ttk.Label(parent, textvariable=self._status_var).pack(anchor=tk.W, pady=(8, 2))

        log_frame = ttk.LabelFrame(parent, text="主页日志")
        log_frame.pack(fill=tk.BOTH, expand=True)
        self._scan_log = tk.Text(log_frame, height=8, wrap=tk.WORD, state=tk.DISABLED)
        self._scan_log.pack(fill=tk.BOTH, expand=True)

    def _build_session_page(self, parent: ttk.Frame) -> None:
        ttk.Label(parent, textvariable=self._session_info_var, font=("", 11, "bold")).pack(
            anchor=tk.W, pady=(0, 8)
        )

        send_box = ttk.LabelFrame(parent, text="发送 (→ Factory RX)")
        send_box.pack(fill=tk.BOTH, expand=False)

        fmt_row = ttk.Frame(send_box)
        fmt_row.pack(fill=tk.X, padx=4, pady=4)
        ttk.Label(fmt_row, text="发送格式:").pack(side=tk.LEFT)
        ttk.Radiobutton(fmt_row, text="Hex", variable=self._send_fmt, value="hex").pack(side=tk.LEFT)
        ttk.Radiobutton(fmt_row, text="UTF-8", variable=self._send_fmt, value="utf8").pack(
            side=tk.LEFT, padx=(0, 12)
        )
        ttk.Label(fmt_row, text="Write:").pack(side=tk.LEFT)
        ttk.Checkbutton(
            fmt_row, text="有响应 (Write Request)", variable=self._write_response
        ).pack(side=tk.LEFT)

        self._send_text = tk.Text(send_box, height=5, wrap=tk.WORD)
        self._send_text.pack(fill=tk.BOTH, expand=True, padx=4, pady=4)
        self._send_text.insert("1.0", "01 02 03 04")

        send_btn_row = ttk.Frame(send_box)
        send_btn_row.pack(fill=tk.X, padx=4, pady=(0, 6))
        self._btn_send = ttk.Button(send_btn_row, text="发送", command=self._on_send)
        self._btn_send.pack(side=tk.LEFT)
        ttk.Button(send_btn_row, text="清空发送", command=lambda: self._send_text.delete("1.0", tk.END)).pack(
            side=tk.LEFT, padx=6
        )

        recv_box = ttk.LabelFrame(parent, text="接收 (← Factory TX Notify)")
        recv_box.pack(fill=tk.BOTH, expand=True, pady=(8, 0))

        recv_fmt_row = ttk.Frame(recv_box)
        recv_fmt_row.pack(fill=tk.X, padx=4, pady=4)
        ttk.Label(recv_fmt_row, text="接收显示:").pack(side=tk.LEFT)
        ttk.Radiobutton(recv_fmt_row, text="Hex", variable=self._recv_fmt, value="hex").pack(side=tk.LEFT)
        ttk.Radiobutton(recv_fmt_row, text="UTF-8", variable=self._recv_fmt, value="utf8").pack(
            side=tk.LEFT, padx=(0, 12)
        )
        self._btn_pause = ttk.Button(recv_fmt_row, text="暂停接收", command=self._on_toggle_pause)
        self._btn_pause.pack(side=tk.LEFT)
        ttk.Button(recv_fmt_row, text="清空接收", command=self._clear_recv).pack(side=tk.LEFT, padx=6)

        self._recv_text = tk.Text(recv_box, height=12, wrap=tk.WORD, state=tk.DISABLED)
        self._recv_text.pack(fill=tk.BOTH, expand=True, padx=4, pady=4)

        bottom = ttk.Frame(parent)
        bottom.pack(fill=tk.X, pady=(10, 0))
        self._btn_exit = ttk.Button(bottom, text="退出并断开连接", command=self._on_exit_session)
        self._btn_exit.pack(side=tk.RIGHT)

    def _build_ota_page(self, parent: ttk.Frame) -> None:
        ttk.Label(parent, textvariable=self._ota_info_var, font=("", 11, "bold")).pack(
            anchor=tk.W, pady=(0, 8)
        )

        file_box = ttk.LabelFrame(parent, text="资源文件")
        file_box.pack(fill=tk.X)
        row1 = ttk.Frame(file_box)
        row1.pack(fill=tk.X, padx=4, pady=4)
        ttk.Label(row1, text="路径:").pack(side=tk.LEFT)
        ttk.Entry(row1, textvariable=self._ota_path_var).pack(
            side=tk.LEFT, fill=tk.X, expand=True, padx=4
        )
        ttk.Button(row1, text="浏览…", command=self._on_browse_ota).pack(side=tk.LEFT)

        row2 = ttk.Frame(file_box)
        row2.pack(fill=tk.X, padx=4, pady=(0, 4))
        ttk.Label(row2, text="img_id:").pack(side=tk.LEFT)
        ttk.Entry(row2, textvariable=self._ota_img_id_var, width=12).pack(side=tk.LEFT, padx=4)
        ttk.Label(row2, text="version:").pack(side=tk.LEFT, padx=(12, 0))
        ttk.Entry(row2, textvariable=self._ota_version_var, width=14).pack(side=tk.LEFT, padx=4)
        ttk.Label(file_box, textvariable=self._ota_file_info_var).pack(anchor=tk.W, padx=4, pady=(0, 6))

        btn_row = ttk.Frame(parent)
        btn_row.pack(fill=tk.X, pady=8)
        self._btn_ota_start = ttk.Button(btn_row, text="启动 OTA", command=self._on_ota_start)
        self._btn_ota_start.pack(side=tk.LEFT)
        self._btn_ota_stop = ttk.Button(
            btn_row, text="停止", command=self._on_ota_stop, state=tk.DISABLED
        )
        self._btn_ota_stop.pack(side=tk.LEFT, padx=6)
        self._btn_ota_exit = ttk.Button(btn_row, text="退出并断开连接", command=self._on_exit_ota)
        self._btn_ota_exit.pack(side=tk.RIGHT)

        prog = ttk.Frame(parent)
        prog.pack(fill=tk.X)
        self._ota_bar = ttk.Progressbar(prog, mode="determinate", maximum=100)
        self._ota_bar.pack(fill=tk.X, pady=(0, 4))
        ttk.Label(prog, textvariable=self._ota_progress_var).pack(anchor=tk.W)
        ttk.Label(prog, textvariable=self._ota_stage_var).pack(anchor=tk.W)

        log_frame = ttk.LabelFrame(parent, text="流程日志")
        log_frame.pack(fill=tk.BOTH, expand=True, pady=(8, 0))
        self._ota_log = tk.Text(log_frame, height=14, wrap=tk.WORD, state=tk.DISABLED)
        self._ota_log.pack(fill=tk.BOTH, expand=True)

    def _hide_all_pages(self) -> None:
        self._scan_frame.pack_forget()
        self._session_frame.pack_forget()
        self._ota_frame.pack_forget()

    def _show_scan_page(self) -> None:
        self._hide_all_pages()
        self._scan_frame.pack(fill=tk.BOTH, expand=True)
        self._update_scan_buttons()

    def _show_session_page(self, address: str, name: str) -> None:
        self._hide_all_pages()
        disp_name = name or "(unnamed)"
        self._session_info_var.set(f"已连接: {disp_name}  ({address})    Factory ✓")
        self._session_frame.pack(fill=tk.BOTH, expand=True)
        self._btn_send.configure(state=tk.NORMAL)
        self._btn_exit.configure(state=tk.NORMAL)

    def _show_ota_page(self, address: str, name: str) -> None:
        self._hide_all_pages()
        disp_name = name or "(unnamed)"
        self._ota_info_var.set(f"OTA 测试  已连接: {disp_name}  ({address})    Remote ✓")
        self._ota_frame.pack(fill=tk.BOTH, expand=True)
        self._update_ota_buttons()

    # --------------------------------------------------------------- helpers
    def _append_scan_log(self, msg: str) -> None:
        ts = datetime.now().strftime("%H:%M:%S")
        line = f"[{ts}] {msg}\n"
        self._scan_log.configure(state=tk.NORMAL)
        self._scan_log.insert(tk.END, line)
        self._scan_log.see(tk.END)
        self._scan_log.configure(state=tk.DISABLED)
        print(line, end="")

    def _append_ota_log(self, msg: str) -> None:
        ts = datetime.now().strftime("%H:%M:%S")
        line = f"[{ts}] {msg}\n"
        self._ota_log.configure(state=tk.NORMAL)
        self._ota_log.insert(tk.END, line)
        self._ota_log.see(tk.END)
        self._ota_log.configure(state=tk.DISABLED)
        print(line, end="")

    def _clear_ota_log(self) -> None:
        self._ota_log.configure(state=tk.NORMAL)
        self._ota_log.delete("1.0", tk.END)
        self._ota_log.configure(state=tk.DISABLED)

    def _append_recv(self, data: bytes) -> None:
        if self._recv_paused.get():
            self._paused_count += 1
            return
        ts = datetime.now().strftime("%H:%M:%S")
        if self._recv_fmt.get() == "utf8":
            body = format_utf8(data)
        else:
            body = format_hex(data)
        line = f"[{ts}] RX ({len(data)}B): {body}\n"
        self._recv_text.configure(state=tk.NORMAL)
        self._recv_text.insert(tk.END, line)
        self._recv_text.see(tk.END)
        self._recv_text.configure(state=tk.DISABLED)
        print(line, end="")

    def _clear_recv(self) -> None:
        self._recv_text.configure(state=tk.NORMAL)
        self._recv_text.delete("1.0", tk.END)
        self._recv_text.configure(state=tk.DISABLED)
        self._paused_count = 0

    def _clear_session(self) -> None:
        self._clear_recv()

    def _reset_ota_ui(self) -> None:
        self._ota_bar["value"] = 0
        self._ota_progress_var.set("进度: —")
        self._ota_stage_var.set("阶段: IDLE")

    def _set_status(self, text: str) -> None:
        self._status_var.set(f"状态: {text}")

    def _update_scan_buttons(self) -> None:
        st = self._state
        has_sel = bool(self._selected_addr)
        if st == "IDLE":
            self._btn_scan.configure(state=tk.NORMAL)
            self._btn_stop.configure(state=tk.DISABLED)
            self._btn_connect.configure(state=tk.NORMAL if has_sel else tk.DISABLED)
            self._btn_ota.configure(state=tk.NORMAL if has_sel else tk.DISABLED)
        elif st == "SCANNING":
            self._btn_scan.configure(state=tk.DISABLED)
            self._btn_stop.configure(state=tk.NORMAL)
            self._btn_connect.configure(state=tk.NORMAL if has_sel else tk.DISABLED)
            self._btn_ota.configure(state=tk.NORMAL if has_sel else tk.DISABLED)
        elif st == "CONNECTING":
            self._btn_scan.configure(state=tk.DISABLED)
            self._btn_stop.configure(state=tk.DISABLED)
            self._btn_connect.configure(state=tk.DISABLED)
            self._btn_ota.configure(state=tk.DISABLED)
        else:
            self._btn_scan.configure(state=tk.DISABLED)
            self._btn_stop.configure(state=tk.DISABLED)
            self._btn_connect.configure(state=tk.DISABLED)
            self._btn_ota.configure(state=tk.DISABLED)

    def _update_ota_buttons(self) -> None:
        st = self._state
        if st == "OTA_RUNNING":
            self._btn_ota_start.configure(state=tk.DISABLED)
            self._btn_ota_stop.configure(state=tk.NORMAL)
            self._btn_ota_exit.configure(state=tk.DISABLED)
        elif st == "OTA_SESSION":
            self._btn_ota_start.configure(state=tk.NORMAL)
            self._btn_ota_stop.configure(state=tk.DISABLED)
            self._btn_ota_exit.configure(state=tk.NORMAL)
        elif st == "DISCONNECTING":
            self._btn_ota_start.configure(state=tk.DISABLED)
            self._btn_ota_stop.configure(state=tk.DISABLED)
            self._btn_ota_exit.configure(state=tk.DISABLED)
        else:
            self._btn_ota_start.configure(state=tk.DISABLED)
            self._btn_ota_stop.configure(state=tk.DISABLED)
            self._btn_ota_exit.configure(state=tk.DISABLED)

    def _refresh_device_list(self) -> None:
        filt = self._filter_var.get().strip().lower()
        selected = self._selected_addr
        for iid in self._tree.get_children():
            self._tree.delete(iid)
        items = list(self._devices.items())

        def sort_key(item: Tuple[str, Dict]) -> Tuple[int, int]:
            rssi = item[1].get("rssi")
            return (0 if rssi is not None else 1, -(rssi or 0))

        items.sort(key=sort_key)
        for addr, info in items:
            name = info.get("name") or ""
            if filt and filt not in name.lower() and filt not in addr.lower():
                continue
            rssi = info.get("rssi")
            rssi_s = "" if rssi is None else str(rssi)
            iid = self._tree.insert("", tk.END, values=(name or "(unnamed)", addr, rssi_s))
            info["iid"] = iid
            if selected and addr == selected:
                self._tree.selection_set(iid)

    # -------------------------------------------------------------- commands
    def _post(self, cmd_type: CmdType, **payload) -> None:
        self._cmd_q.put(Cmd(cmd_type, payload))

    def _on_scan_start(self) -> None:
        self._devices.clear()
        self._refresh_device_list()
        self._state = "SCANNING"
        self._set_status("扫描中…")
        self._update_scan_buttons()
        self._append_scan_log("scan start")
        self._post(CmdType.SCAN_START)

    def _on_scan_stop(self) -> None:
        self._post(CmdType.SCAN_STOP)

    def _on_tree_select(self, _event=None) -> None:
        sel = self._tree.selection()
        if not sel:
            self._selected_addr = None
            self._selected_var.set("选中: (无)")
        else:
            vals = self._tree.item(sel[0], "values")
            self._selected_addr = vals[1]
            self._selected_var.set(f"选中: {vals[0]}  {vals[1]}")
        self._update_scan_buttons()

    def _on_connect(self) -> None:
        if self._state not in ("IDLE", "SCANNING"):
            self._append_scan_log(f"忽略连接请求（当前状态 {self._state}）")
            return
        if not self._selected_addr:
            messagebox.showwarning(APP_TITLE, "请先在列表中选中设备")
            return
        addr = self._selected_addr
        name = self._devices.get(addr, {}).get("name", "")
        self._state = "CONNECTING"
        self._set_status(f"连接中 {addr} …")
        self._update_scan_buttons()
        self._append_scan_log(f"connecting Factory {name} ({addr})")
        self._post(CmdType.SCAN_STOP)
        self._post(CmdType.CONNECT, address=addr, name=name)

    def _on_ota_connect(self) -> None:
        if self._state not in ("IDLE", "SCANNING"):
            self._append_scan_log(f"忽略 OTA 连接请求（当前状态 {self._state}）")
            return
        if not self._selected_addr:
            messagebox.showwarning(APP_TITLE, "请先在列表中选中设备")
            return
        addr = self._selected_addr
        name = self._devices.get(addr, {}).get("name", "")
        self._state = "CONNECTING"
        self._set_status(f"OTA 连接中 {addr} …")
        self._update_scan_buttons()
        self._append_scan_log(f"connecting OTA/Remote {name} ({addr})")
        self._post(CmdType.SCAN_STOP)
        self._post(CmdType.OTA_CONNECT, address=addr, name=name)

    def _on_browse_ota(self) -> None:
        path = filedialog.askopenfilename(
            title="选择图片资源文件",
            filetypes=[("Binary / All", "*.*"), ("Bin", "*.bin"), ("Image pack", "*.img")],
        )
        if not path:
            return
        try:
            meta = load_ota_file(path)
        except Exception as exc:
            messagebox.showerror(APP_TITLE, str(exc))
            return
        self._ota_file_meta = meta
        self._ota_path_var.set(path)
        self._ota_file_info_var.set(
            f"文件信息: size={meta.size}  CRC32=0x{meta.crc32:08X}  ({Path(path).name})"
        )
        self._append_ota_log(
            f"file loaded {Path(path).name} size={meta.size} crc=0x{meta.crc32:08X}"
        )

    def _on_ota_start(self) -> None:
        if self._state != "OTA_SESSION":
            return
        path = self._ota_path_var.get().strip()
        if not path:
            messagebox.showwarning(APP_TITLE, "请先选择资源文件")
            return
        try:
            img_id = parse_int_field(self._ota_img_id_var.get(), "img_id")
            version = parse_int_field(self._ota_version_var.get(), "version")
        except ValueError as exc:
            messagebox.showerror(APP_TITLE, str(exc))
            return
        # Refresh meta if path typed manually
        try:
            if self._ota_file_meta is None or self._ota_file_meta.path != path:
                self._ota_file_meta = load_ota_file(path)
                m = self._ota_file_meta
                self._ota_file_info_var.set(
                    f"文件信息: size={m.size}  CRC32=0x{m.crc32:08X}  ({Path(path).name})"
                )
        except Exception as exc:
            messagebox.showerror(APP_TITLE, str(exc))
            return

        self._state = "OTA_RUNNING"
        self._update_ota_buttons()
        self._append_ota_log(f"启动 OTA img_id={img_id} version=0x{version:08X}")
        self._post(CmdType.OTA_START, path=path, img_id=img_id, version=version)

    def _on_ota_stop(self) -> None:
        if self._state != "OTA_RUNNING":
            return
        self._append_ota_log("请求停止…")
        self._post(CmdType.OTA_STOP)

    def _on_send(self) -> None:
        raw = self._send_text.get("1.0", tk.END)
        try:
            if self._send_fmt.get() == "utf8":
                data = raw.rstrip("\n").encode("utf-8")
                if not data:
                    raise ValueError("发送内容为空")
            else:
                data = parse_hex_payload(raw)
        except ValueError as exc:
            messagebox.showerror(APP_TITLE, str(exc))
            return
        self._post(
            CmdType.WRITE_RX,
            data=data,
            response=bool(self._write_response.get()),
        )

    def _on_toggle_pause(self) -> None:
        paused = not self._recv_paused.get()
        self._recv_paused.set(paused)
        if paused:
            self._btn_pause.configure(text="继续接收")
            self._append_recv_sys("接收已暂停（包仍接收，仅不刷新 UI）")
        else:
            self._btn_pause.configure(text="暂停接收")
            skipped = self._paused_count
            self._paused_count = 0
            self._append_recv_sys(f"接收已恢复（暂停期间跳过 UI 刷新 {skipped} 包）")

    def _append_recv_sys(self, msg: str) -> None:
        ts = datetime.now().strftime("%H:%M:%S")
        line = f"[{ts}] {msg}\n"
        self._recv_text.configure(state=tk.NORMAL)
        self._recv_text.insert(tk.END, line)
        self._recv_text.see(tk.END)
        self._recv_text.configure(state=tk.DISABLED)

    def _on_exit_session(self) -> None:
        if self._state == "DISCONNECTING":
            return
        self._state = "DISCONNECTING"
        self._btn_exit.configure(state=tk.DISABLED)
        self._btn_send.configure(state=tk.DISABLED)
        self._post(CmdType.DISCONNECT)

    def _on_exit_ota(self) -> None:
        if self._state == "DISCONNECTING":
            return
        if self._state == "OTA_RUNNING":
            self._post(CmdType.OTA_STOP)
        self._state = "DISCONNECTING"
        self._update_ota_buttons()
        self._post(CmdType.DISCONNECT)

    # --------------------------------------------------------------- events
    def _poll_events(self) -> None:
        try:
            while True:
                evt: Evt = self._evt_q.get_nowait()
                self._handle_evt(evt)
        except queue.Empty:
            pass
        self.after(100, self._poll_events)

    def _handle_evt(self, evt: Evt) -> None:
        t = evt.type
        p = evt.payload
        if t == EvtType.DEVICE_FOUND:
            addr = p.get("address", "")
            if not addr:
                return
            info = self._devices.get(addr, {})
            name = p.get("name") or info.get("name") or ""
            rssi = p.get("rssi")
            if rssi is None:
                rssi = info.get("rssi")
            self._devices[addr] = {"name": name, "rssi": rssi}
            self._refresh_device_list()
        elif t == EvtType.SCAN_STOPPED:
            if self._state == "SCANNING":
                self._state = "IDLE"
                self._set_status("空闲")
                self._update_scan_buttons()
            self._append_scan_log("scan stopped")
        elif t == EvtType.CONNECTING:
            self._append_scan_log(f"connecting… {p.get('address')}")
        elif t == EvtType.FACTORY_ENABLED:
            addr = p.get("address", "")
            name = p.get("name", "")
            self._state = "SESSION"
            self._append_scan_log(f"Factory enabled: {name} ({addr})")
            self._show_session_page(addr, name)
        elif t == EvtType.OTA_READY:
            addr = p.get("address", "")
            name = p.get("name", "")
            self._state = "OTA_SESSION"
            self._clear_ota_log()
            self._reset_ota_ui()
            self._append_scan_log(f"OTA Remote ready: {name} ({addr})")
            self._append_ota_log("connect OK, Remote TX notify enabled")
            self._show_ota_page(addr, name)
        elif t == EvtType.OTA_LOG:
            self._append_ota_log(p.get("message", ""))
        elif t == EvtType.OTA_PROGRESS:
            offset = int(p.get("offset", 0))
            total = int(p.get("total", 0)) or 1
            stage = p.get("stage", "")
            pct = min(100, int(offset * 100 / total))
            self._ota_bar["value"] = pct
            self._ota_progress_var.set(f"进度: {pct}%   已传 {offset} / {total}")
            self._ota_stage_var.set(f"阶段: {stage}")
        elif t == EvtType.OTA_FINISHED:
            success = bool(p.get("success"))
            msg = p.get("message", "")
            if self._state == "OTA_RUNNING":
                self._state = "OTA_SESSION"
            self._update_ota_buttons()
            self._append_ota_log(f"完成: {'成功' if success else '失败/停止'} — {msg}")
            if success:
                messagebox.showinfo(APP_TITLE, f"OTA 成功\n{msg}")
            elif msg and msg != "已停止":
                messagebox.showerror(APP_TITLE, f"OTA 失败\n{msg}")
        elif t == EvtType.NOTIFY_DATA:
            data = p.get("data", b"")
            if isinstance(data, (bytes, bytearray)):
                self._append_recv(bytes(data))
        elif t == EvtType.WRITE_DONE:
            self._append_recv_sys(f"已发送 {p.get('length', 0)} 字节")
        elif t == EvtType.DISCONNECTED:
            was_sub = self._state in (
                "SESSION",
                "OTA_SESSION",
                "OTA_RUNNING",
                "DISCONNECTING",
                "CONNECTING",
            )
            was_ota = self._state in ("OTA_SESSION", "OTA_RUNNING", "DISCONNECTING")
            self._state = "IDLE"
            self._clear_session()
            self._reset_ota_ui()
            self._recv_paused.set(False)
            self._btn_pause.configure(text="暂停接收")
            self._show_scan_page()
            self._set_status("已断开")
            self._update_scan_buttons()
            reason = p.get("reason", "")
            self._append_scan_log(f"disconnected ({reason})")
            if was_sub and reason == "remote_or_lost":
                messagebox.showwarning(APP_TITLE, "设备连接已断开")
            elif was_ota:
                pass
        elif t == EvtType.ERROR:
            msg = p.get("message", "未知错误")
            self._append_scan_log(f"ERROR: {msg}")
            print(f"ERROR: {msg}")
            if self._state == "CONNECTING":
                self._state = "IDLE"
                self._set_status("连接失败")
                self._update_scan_buttons()
                messagebox.showerror(APP_TITLE, msg)
            elif self._state in ("SESSION", "OTA_SESSION"):
                messagebox.showerror(APP_TITLE, msg)
            elif self._state == "OTA_RUNNING":
                self._append_ota_log(f"ERROR: {msg}")
                messagebox.showerror(APP_TITLE, msg)
            elif self._state == "DISCONNECTING":
                self._state = "IDLE"
                self._show_scan_page()
                self._set_status("断开异常")
                self._update_scan_buttons()
            else:
                messagebox.showerror(APP_TITLE, msg)
        elif t == EvtType.LOG:
            msg = p.get("message", "")
            if self._state in ("OTA_SESSION", "OTA_RUNNING"):
                # Keep scan log too for connect diagnostics
                self._append_scan_log(msg)
            else:
                self._append_scan_log(msg)

    def _on_close(self) -> None:
        try:
            self._post(CmdType.OTA_STOP)
            self._post(CmdType.DISCONNECT)
            self._post(CmdType.SHUTDOWN)
            self._worker.stop(timeout=3.0)
        except Exception:
            LOG.exception("shutdown")
        self.destroy()


def run_app() -> None:
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
    )
    app = BleFactoryApp()
    app.mainloop()
