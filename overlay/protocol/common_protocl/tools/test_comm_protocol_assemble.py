#!/usr/bin/env python3
"""
comm_protocol 组包 / 拆包测试 + 图片资源 OTA 全流程模拟

1. 验证 comm_protocol_parser.c 环形缓冲组包（含 BLE MTU 分片）
2. 模拟上位机与设备端完整 OTA 交互（原子 TLV 协议 v2.x）

用法:
    python3 test_comm_protocol_assemble.py           # 运行全部测试
    python3 test_comm_protocol_assemble.py -v          # 打印 OTA 逐步细节
    python3 test_comm_protocol_assemble.py --list    # 列出用例与覆盖范围
    python3 test_comm_protocol_assemble.py --demo      # 打印 OTA 帧十六进制
"""

from __future__ import annotations

import argparse
import struct
import sys
import zlib
from dataclasses import dataclass, field
from typing import Callable, Dict, List, Optional, Tuple

# ── 协议常量（与 comm_protocol_defs.h 一致）────────────────────────────
COMM_PROTOCOL_SOF = 0x5CC5
COMM_PROTOCOL_HEADER_SIZE = 6
COMM_PROTOCOL_FOOTER_SIZE = 2
COMM_PROTOCOL_SERVICE_HEADER_SIZE = 4
COMM_PROTOCOL_TLV_HEADER_SIZE = 4
COMM_PROTOCOL_PAYLOAD_SIZE_MIN = COMM_PROTOCOL_HEADER_SIZE + COMM_PROTOCOL_FOOTER_SIZE
COMM_PROTOCOL_MAX_PAYLOAD_SIZE = 1024 + 64  # 与 comm_protocol_defs.h 一致
COMM_PROTOCOL_OTA_SERVICE = 9
PROTOCOL_SERVICE_ERROR_CODE_TLV_ID = 0xF000
FRAME_TYPE_REQUEST = 0
FRAME_TYPE_RESPONSE = 1

PACKET_ASSEMBLY_BUFFER_SIZE = COMM_PROTOCOL_MAX_PAYLOAD_SIZE * 2  # 与 comm_protocol_parser.c 一致
PACKET_ASSEMBLY_MAX_SYNC_STEPS = 256
PACKET_ASSEMBLY_SOF_BYTE0 = 0xC5
PACKET_ASSEMBLY_SOF_BYTE1 = 0x5C

VERBOSE = False


def log_step(msg: str) -> None:
    if VERBOSE:
        print(f"    {msg}")

# ── OTA TLV（与 ota_tlvs_protocol.h 一致）────────────────────────────
OTA_TLV_IMG_ID = 0x0001
OTA_TLV_VERSION = 0x0002
OTA_TLV_CRC32 = 0x0003
OTA_TLV_TOTAL_SIZE = 0x0004
OTA_TLV_TYPE = 0x0005
OTA_TLV_BLOCK_SIZE = 0x0006
OTA_TLV_START_UPGRADE_REQUEST = 0x0007
OTA_TLV_IMG_OFFSET_ADDR = 0x0008
OTA_TLV_WRITE_IMG_DATA = 0x0009
OTA_TLV_END_UPGRADE_REQUEST = 0x000A
OTA_TLV_UPGRADE_RESULT = 0x000B
OTA_TLV_UPGRADE_STATUS_NOTIFY = 0x000C

OTA_OBJECT_IMAGE_RESOURCE = 0x02
OTA_BLOCK_SIZE = 1024
OTA_STATUS_SUCCESS = 0x0000
OTA_STATUS_FAIL = 0x0001


def crc16_compute(data: bytes, crc: int = 0xFFFF) -> int:
    """与 trunk/root_source/utils/gernal_utils.c 中 crc16_compute 一致"""
    for b in data:
        crc = ((crc >> 8) & 0xFFFF) | ((crc << 8) & 0xFFFF)
        crc ^= b
        crc ^= (crc & 0xFF) >> 4
        crc ^= ((crc << 8) << 4) & 0xFFFF
        crc ^= (((crc & 0xFF) << 4) << 1) & 0xFFFF
    return crc & 0xFFFF


def crc32_resource(data: bytes) -> int:
    """标准 CRC32，与固件 img_res_integrity_check 一致"""
    return zlib.crc32(data) & 0xFFFFFFFF


def ota_max_img_data_per_frame() -> int:
    """单帧 Payload 上限内可容纳的最大 IMG_DATA 长度（原子 TLV 布局）"""
    return (
        COMM_PROTOCOL_MAX_PAYLOAD_SIZE
        - COMM_PROTOCOL_SERVICE_HEADER_SIZE
        - (COMM_PROTOCOL_TLV_HEADER_SIZE + 4)  # IMG_OFFSET_ADDR
        - COMM_PROTOCOL_TLV_HEADER_SIZE
    )


# ── 组包 ──────────────────────────────────────────────────────────────


def build_tlv(tlv_type: int, value: bytes) -> bytes:
    return struct.pack("<HH", tlv_type, len(value)) + value


def build_tlv_u32(tlv_type: int, value: int) -> bytes:
    return build_tlv(tlv_type, struct.pack("<I", value))


def build_tlv_u16(tlv_type: int, value: int) -> bytes:
    return build_tlv(tlv_type, struct.pack("<H", value))


def build_tlv_u8(tlv_type: int, value: int) -> bytes:
    return build_tlv(tlv_type, struct.pack("<B", value))


def build_tlv_empty(tlv_type: int) -> bytes:
    return build_tlv(tlv_type, b"")


def build_ota_service(tlvs: bytes) -> bytes:
    return struct.pack("<HH", COMM_PROTOCOL_OTA_SERVICE, len(tlvs)) + tlvs


def build_frame(seq: int, frame_type: int, payload: bytes) -> bytes:
    if len(payload) > COMM_PROTOCOL_MAX_PAYLOAD_SIZE:
        raise ValueError(f"payload 超长: {len(payload)} > {COMM_PROTOCOL_MAX_PAYLOAD_SIZE}")
    header = struct.pack("<HBBH", COMM_PROTOCOL_SOF, seq, frame_type, len(payload))
    body = header + payload
    return body + struct.pack("<H", crc16_compute(body))


def build_ota_request_frame(seq: int, tlvs: bytes) -> bytes:
    return build_frame(seq, FRAME_TYPE_REQUEST, build_ota_service(tlvs))


def build_ota_start_upgrade_frame(
    seq: int,
    img_id: int,
    version: int,
    crc32: int,
    total_size: int,
    ota_type: int = OTA_OBJECT_IMAGE_RESOURCE,
) -> bytes:
    tlvs = (
        build_tlv_u32(OTA_TLV_IMG_ID, img_id)
        + build_tlv_u32(OTA_TLV_VERSION, version)
        + build_tlv_u32(OTA_TLV_CRC32, crc32)
        + build_tlv_u32(OTA_TLV_TOTAL_SIZE, total_size)
        + build_tlv_u8(OTA_TLV_TYPE, ota_type)
        + build_tlv_empty(OTA_TLV_START_UPGRADE_REQUEST)
    )
    return build_ota_request_frame(seq, tlvs)


def build_ota_write_data_frame(seq: int, offset: int, data: bytes) -> bytes:
    tlvs = build_tlv_u32(OTA_TLV_IMG_OFFSET_ADDR, offset) + build_tlv(
        OTA_TLV_WRITE_IMG_DATA, data
    )
    return build_ota_request_frame(seq, tlvs)


def build_ota_end_upgrade_frame(seq: int) -> bytes:
    return build_ota_request_frame(seq, build_tlv_empty(OTA_TLV_END_UPGRADE_REQUEST))


# ── 拆包 ──────────────────────────────────────────────────────────────


@dataclass
class ParsedTlv:
    tlv_type: int
    value: bytes


@dataclass
class ParsedService:
    svr_id: int
    tlvs: List[ParsedTlv]


@dataclass
class ParsedFrame:
    seq: int
    frame_type: int
    payload: bytes
    services: List[ParsedService]
    raw: bytes


def parse_frame(frame: bytes) -> ParsedFrame:
    if len(frame) < COMM_PROTOCOL_HEADER_SIZE + COMM_PROTOCOL_FOOTER_SIZE:
        raise ValueError("帧过短")
    sof, seq, frame_type, payload_len = struct.unpack_from("<HBBH", frame, 0)
    if sof != COMM_PROTOCOL_SOF:
        raise ValueError(f"SOF 错误: {sof:#06x}")
    expected_crc = crc16_compute(frame[: COMM_PROTOCOL_HEADER_SIZE + payload_len])
    received_crc = struct.unpack_from("<H", frame, COMM_PROTOCOL_HEADER_SIZE + payload_len)[0]
    if expected_crc != received_crc:
        raise ValueError(f"CRC16 错误: {received_crc:#06x} != {expected_crc:#06x}")

    payload = frame[COMM_PROTOCOL_HEADER_SIZE : COMM_PROTOCOL_HEADER_SIZE + payload_len]
    services: List[ParsedService] = []
    offset = 0
    while offset < len(payload):
        svr_id, srv_len = struct.unpack_from("<HH", payload, offset)
        offset += COMM_PROTOCOL_SERVICE_HEADER_SIZE
        tlv_blob = payload[offset : offset + srv_len]
        offset += srv_len
        tlvs: List[ParsedTlv] = []
        tlv_off = 0
        while tlv_off < len(tlv_blob):
            tlv_type, tlv_len = struct.unpack_from("<HH", tlv_blob, tlv_off)
            tlv_off += COMM_PROTOCOL_TLV_HEADER_SIZE
            value = tlv_blob[tlv_off : tlv_off + tlv_len]
            tlv_off += tlv_len
            tlvs.append(ParsedTlv(tlv_type, value))
        services.append(ParsedService(svr_id, tlvs))
    return ParsedFrame(seq, frame_type, payload, services, frame)


def get_ota_service(frame: ParsedFrame) -> ParsedService:
    for svc in frame.services:
        if svc.svr_id == COMM_PROTOCOL_OTA_SERVICE:
            return svc
    raise ValueError("未找到 OTA Service")


def get_tlv(service: ParsedService, tlv_type: int) -> Optional[ParsedTlv]:
    for tlv in service.tlvs:
        if tlv.tlv_type == tlv_type:
            return tlv
    return None


def get_tlv_u32(service: ParsedService, tlv_type: int) -> int:
    tlv = get_tlv(service, tlv_type)
    if tlv is None or len(tlv.value) != 4:
        raise ValueError(f"TLV {tlv_type:#06x} 缺失或长度错误")
    return struct.unpack("<I", tlv.value)[0]


def get_tlv_u16(service: ParsedService, tlv_type: int) -> int:
    tlv = get_tlv(service, tlv_type)
    if tlv is None or len(tlv.value) != 2:
        raise ValueError(f"TLV {tlv_type:#06x} 缺失或长度错误")
    return struct.unpack("<H", tlv.value)[0]


# ── 组包缓冲（对齐 comm_protocol_parser.c）──────────────────────────


def split_chunks(data: bytes, mtu: int) -> List[bytes]:
    return [data[i : i + mtu] for i in range(0, len(data), mtu)]


@dataclass
class PacketAssemblyBuffer:
    buffer: bytearray = field(default_factory=lambda: bytearray(PACKET_ASSEMBLY_BUFFER_SIZE))
    write_pos: int = 0
    read_pos: int = 0
    data_size: int = 0

    def reset(self) -> None:
        self.write_pos = 0
        self.read_pos = 0
        self.data_size = 0

    def write(self, data: bytes) -> int:
        if len(data) > PACKET_ASSEMBLY_BUFFER_SIZE - self.data_size:
            return -1
        for b in data:
            self.buffer[self.write_pos] = b
            self.write_pos = (self.write_pos + 1) % PACKET_ASSEMBLY_BUFFER_SIZE
        self.data_size += len(data)
        return 0

    def peek(self, offset: int) -> int:
        return self.buffer[(self.read_pos + offset) % PACKET_ASSEMBLY_BUFFER_SIZE]

    def peek_bytes(self, offset: int, length: int) -> bytes:
        return bytes(self.peek(offset + i) for i in range(length))

    def read(self, length: int) -> bytes:
        out = bytearray(length)
        for i in range(length):
            out[i] = self.buffer[self.read_pos]
            self.read_pos = (self.read_pos + 1) % PACKET_ASSEMBLY_BUFFER_SIZE
        self.data_size -= length
        return bytes(out)


def packet_assembly_sync_to_sof(asm: PacketAssemblyBuffer) -> None:
    while asm.data_size > 0:
        if asm.peek(0) == PACKET_ASSEMBLY_SOF_BYTE0:
            if asm.data_size < 2:
                return
            if asm.peek(1) == PACKET_ASSEMBLY_SOF_BYTE1:
                return
        asm.read(1)


def comm_protocol_assemble_packet(
    asm: PacketAssemblyBuffer,
    data: bytes,
    callback: Callable[[bytes], None],
) -> int:
    if not data:
        return -1
    if asm.write(data) < 0:
        asm.reset()
        return -2

    sync_steps = 0
    while asm.data_size > 0:
        if sync_steps >= PACKET_ASSEMBLY_MAX_SYNC_STEPS:
            return 1
        if asm.data_size < COMM_PROTOCOL_HEADER_SIZE:
            return 1

        header = asm.peek_bytes(0, COMM_PROTOCOL_HEADER_SIZE)
        sof = header[0] | (header[1] << 8)
        if sof != COMM_PROTOCOL_SOF:
            packet_assembly_sync_to_sof(asm)
            sync_steps += 1
            continue

        payload_length = header[4] | (header[5] << 8)
        if payload_length < COMM_PROTOCOL_PAYLOAD_SIZE_MIN or payload_length > COMM_PROTOCOL_MAX_PAYLOAD_SIZE:
            asm.read(1)
            sync_steps += 1
            continue

        frame_size = COMM_PROTOCOL_HEADER_SIZE + payload_length + COMM_PROTOCOL_FOOTER_SIZE
        if asm.data_size < frame_size:
            return 1

        frame = asm.peek_bytes(0, frame_size)
        checksum_expected = crc16_compute(frame[:-COMM_PROTOCOL_FOOTER_SIZE])
        checksum_received = frame[-2] | (frame[-1] << 8)
        if checksum_expected != checksum_received:
            asm.read(1)
            sync_steps += 1
            continue

        asm.read(frame_size)
        callback(frame)

    return 0


def feed_chunks(chunks: List[bytes], callback: Callable[[bytes], None]) -> Tuple[List[bytes], List[int]]:
    asm = PacketAssemblyBuffer()
    frames: List[bytes] = []
    rets: List[int] = []

    def _cb(frame: bytes) -> None:
        frames.append(frame)

    for chunk in chunks:
        ret = comm_protocol_assemble_packet(asm, chunk, _cb)
        rets.append(ret)

    return frames, rets


def transceive_via_mtu(request: bytes, mtu: int, handler: Callable[[bytes], bytes]) -> bytes:
    """模拟 BLE 分片发送请求、组包后交给 handler，响应同样经组包返回"""
    asm = PacketAssemblyBuffer()
    return transceive_via_mtu_persistent(asm, request, mtu, handler)


def transceive_via_mtu_persistent(
    asm: PacketAssemblyBuffer,
    request: bytes,
    mtu: int,
    handler: Callable[[bytes], bytes],
    garbage_before: Optional[bytes] = None,
) -> bytes:
    """复用同一组包缓冲；可在本次请求前注入脏字节（模拟块与块之间的线路噪声）"""
    rsp_holder: List[bytes] = []

    def on_frame(frame: bytes) -> None:
        rsp_holder.append(handler(frame))

    if garbage_before:
        comm_protocol_assemble_packet(asm, garbage_before, on_frame)

    for chunk in split_chunks(request, mtu):
        comm_protocol_assemble_packet(asm, chunk, on_frame)

    if not rsp_holder:
        raise RuntimeError("未收到完整请求帧")
    return rsp_holder[-1]


# ── 模拟设备端 OTA 状态机 ─────────────────────────────────────────────


@dataclass
class MockOtaDevice:
    """对齐 ota_tlvs_protocol.c + img_res_ota.c 的简化行为"""

    img_id: int = 0
    version: int = 0
    crc32: int = 0
    total_size: int = 0
    ota_type: int = 0
    offset_addr: int = 0
    block_size: int = OTA_BLOCK_SIZE
    started: bool = False
    buffer: bytearray = field(default_factory=bytearray)

    def reset_session(self) -> None:
        self.img_id = 0
        self.version = 0
        self.crc32 = 0
        self.total_size = 0
        self.ota_type = 0
        self.offset_addr = 0
        self.started = False
        self.buffer.clear()

    def handle_request(self, frame_bytes: bytes) -> bytes:
        req = parse_frame(frame_bytes)
        if req.frame_type != FRAME_TYPE_REQUEST:
            raise ValueError("非请求帧")

        session_fields: Dict[str, int] = {}
        action: Optional[str] = None
        write_offset: Optional[int] = None
        write_data: Optional[bytes] = None

        for tlv in get_ota_service(req).tlvs:
            if tlv.tlv_type == OTA_TLV_IMG_ID:
                session_fields["img_id"] = struct.unpack("<I", tlv.value)[0]
            elif tlv.tlv_type == OTA_TLV_VERSION:
                session_fields["version"] = struct.unpack("<I", tlv.value)[0]
            elif tlv.tlv_type == OTA_TLV_CRC32:
                session_fields["crc32"] = struct.unpack("<I", tlv.value)[0]
            elif tlv.tlv_type == OTA_TLV_TOTAL_SIZE:
                session_fields["total_size"] = struct.unpack("<I", tlv.value)[0]
            elif tlv.tlv_type == OTA_TLV_TYPE:
                session_fields["ota_type"] = tlv.value[0]
            elif tlv.tlv_type == OTA_TLV_IMG_OFFSET_ADDR:
                write_offset = struct.unpack("<I", tlv.value)[0]
            elif tlv.tlv_type == OTA_TLV_WRITE_IMG_DATA:
                write_data = tlv.value
            elif tlv.tlv_type == OTA_TLV_START_UPGRADE_REQUEST:
                action = "start"
            elif tlv.tlv_type == OTA_TLV_END_UPGRADE_REQUEST:
                action = "end"

        for key, val in session_fields.items():
            setattr(self, key, val)

        rsp_tlvs = b""
        err_code = 0

        if action == "start":
            if self.ota_type != OTA_OBJECT_IMAGE_RESOURCE:
                err_code = 1
            else:
                self.started = True
                self.buffer.clear()
                resume_offset = 0
                rsp_tlvs = (
                    build_tlv_u32(OTA_TLV_IMG_OFFSET_ADDR, resume_offset)
                    + build_tlv_u16(OTA_TLV_BLOCK_SIZE, OTA_BLOCK_SIZE)
                )
        elif action == "end":
            if not self.started:
                err_code = 1
                result = OTA_STATUS_FAIL
            else:
                ok = len(self.buffer) == self.total_size and crc32_resource(bytes(self.buffer)) == self.crc32
                result = OTA_STATUS_SUCCESS if ok else OTA_STATUS_FAIL
                rsp_tlvs = build_tlv_u16(OTA_TLV_UPGRADE_RESULT, result)
                self.reset_session()
        elif write_data is not None:
            if not self.started or write_offset is None:
                err_code = 1
            else:
                end = write_offset + len(write_data)
                if end > len(self.buffer):
                    self.buffer.extend(b"\x00" * (end - len(self.buffer)))
                self.buffer[write_offset : write_offset + len(write_data)] = write_data
        else:
            err_code = 1

        rsp_tlvs += build_tlv_u16(PROTOCOL_SERVICE_ERROR_CODE_TLV_ID, err_code)
        return build_frame(req.seq, FRAME_TYPE_RESPONSE, build_ota_service(rsp_tlvs))


# ── 上位机 OTA 流程（测试用）──────────────────────────────────────────


def run_ota_upgrade(
    device: MockOtaDevice,
    file_data: bytes,
    img_id: int = 1,
    version: int = 0x00010000,
    mtu: int = 512,
) -> None:
    total_size = len(file_data)
    file_crc32 = crc32_resource(file_data)
    seq = 1
    chunk_cap = min(OTA_BLOCK_SIZE, ota_max_img_data_per_frame())
    block_count = 0

    log_step(f"START  img_id={img_id} size={total_size} crc32={file_crc32:#010x} mtu={mtu}")
    req = build_ota_start_upgrade_frame(seq, img_id, version, file_crc32, total_size)
    log_step(f"  请求帧 {len(req)}B, payload={len(req) - 8}B")
    rsp = transceive_via_mtu(req, mtu, device.handle_request)
    parsed = parse_frame(rsp)
    svc = get_ota_service(parsed)
    f000 = get_tlv_u16(svc, PROTOCOL_SERVICE_ERROR_CODE_TLV_ID)
    assert f000 == 0, f"START F000={f000}"
    offset = get_tlv_u32(svc, OTA_TLV_IMG_OFFSET_ADDR)
    block_size = get_tlv_u16(svc, OTA_TLV_BLOCK_SIZE)
    assert block_size == OTA_BLOCK_SIZE
    log_step(f"  应答 offset={offset} block_size={block_size} F000={f000}")

    seq = 2
    while offset < total_size:
        chunk_len = min(chunk_cap, total_size - offset)
        chunk = file_data[offset : offset + chunk_len]
        block_count += 1
        log_step(f"WRITE #{block_count} seq={seq} offset={offset} len={chunk_len}")
        rsp = transceive_via_mtu(
            build_ota_write_data_frame(seq, offset, chunk),
            mtu,
            device.handle_request,
        )
        svc = get_ota_service(parse_frame(rsp))
        f000 = get_tlv_u16(svc, PROTOCOL_SERVICE_ERROR_CODE_TLV_ID)
        assert f000 == 0, f"WRITE F000={f000} at offset={offset}"
        offset += chunk_len
        seq = (seq + 1) & 0xFF

    log_step(f"END    seq={seq} 共写入 {block_count} 块")
    rsp = transceive_via_mtu(build_ota_end_upgrade_frame(seq), mtu, device.handle_request)
    svc = get_ota_service(parse_frame(rsp))
    f000 = get_tlv_u16(svc, PROTOCOL_SERVICE_ERROR_CODE_TLV_ID)
    result = get_tlv_u16(svc, OTA_TLV_UPGRADE_RESULT)
    assert f000 == 0, f"END F000={f000}"
    assert result == OTA_STATUS_SUCCESS, f"UPGRADE_RESULT={result:#06x}"
    log_step(f"  应答 UPGRADE_RESULT={result:#06x} F000={f000}")


# ── 原组包测试 + OTA 测试 ─────────────────────────────────────────────


def test_single_chunk() -> None:
    frame = build_ota_request_frame(1, build_tlv_u32(OTA_TLV_IMG_ID, 1))
    frames, _ = feed_chunks([frame], lambda f: None)
    assert len(frames) == 1 and frames[0] == frame
    print("[PASS] 单包（无分片）")


def test_mtu_split() -> None:
    big_value = bytes(range(256)) * 2
    tlv = struct.pack("<HH", 0x0100, len(big_value)) + big_value
    service = struct.pack("<HH", COMM_PROTOCOL_OTA_SERVICE, len(tlv)) + tlv
    frame = build_frame(2, 0, service)

    mtu = 200
    chunks = split_chunks(frame, mtu)
    frames, rets = feed_chunks(chunks, lambda f: None)

    assert len(frames) == 1 and frames[0] == frame
    assert rets[-1] == 0
    assert any(r == 1 for r in rets[:-1])
    print(f"[PASS] MTU={mtu} 分片组包 (帧长={len(frame)}, 分{len(chunks)}片)")


def test_small_mtu() -> None:
    frame = build_ota_request_frame(3, build_tlv(OTA_TLV_IMG_ID, bytes(100)))
    mtu = 20
    frames, _ = feed_chunks(split_chunks(frame, mtu), lambda f: None)
    assert len(frames) == 1 and frames[0] == frame
    print(f"[PASS] MTU={mtu} 小分片组包 (帧长={len(frame)}, 分{len(split_chunks(frame, mtu))}片)")


def test_two_frames_back_to_back() -> None:
    f1 = build_ota_request_frame(10, build_tlv(OTA_TLV_IMG_ID, b"\xAA"))
    f2 = build_ota_request_frame(11, build_tlv(OTA_TLV_IMG_ID, b"\xBB"))
    frames, _ = feed_chunks(split_chunks(f1 + f2, 64), lambda f: None)
    assert len(frames) == 2 and frames[0] == f1 and frames[1] == f2
    print("[PASS] 连续两帧粘包 + 分片")


def test_invalid_sof_recovery() -> None:
    frame = build_ota_request_frame(20, build_tlv_u32(OTA_TLV_IMG_ID, 1))
    frames, _ = feed_chunks([b"\xFF\xFE\x00" + frame], lambda f: None)
    assert len(frames) == 1 and frames[0] == frame
    print("[PASS] 脏数据前缀 + SOF 滑动恢复")


def test_dirty_data_between_two_write_frames() -> None:
    """两帧 WRITE 请求之间穿插脏字节（含假 SOF 片段），应正确拆出两帧"""
    f1 = build_ota_write_data_frame(2, 0, bytes(128))
    f2 = build_ota_write_data_frame(3, 128, bytes(64))
    garbage = b"\xFF\xFE\x00\x11" + b"\x5C\xC5\x99" + bytes(17)  # 含半包 SOF 噪声
    chunks = split_chunks(f1, 64) + [garbage] + split_chunks(f2, 64)
    frames, _ = feed_chunks(chunks, lambda f: None)
    assert len(frames) == 2 and frames[0] == f1 and frames[1] == f2
    print("[PASS] 两帧 WRITE 之间穿插脏数据仍可组包")


def test_incomplete_then_complete() -> None:
    frame = build_ota_request_frame(30, build_tlv(OTA_TLV_IMG_ID, bytes(80)))
    mid = len(frame) // 2
    asm = PacketAssemblyBuffer()
    got: List[bytes] = []

    r1 = comm_protocol_assemble_packet(asm, frame[:mid], lambda f: got.append(f))
    r2 = comm_protocol_assemble_packet(asm, frame[mid:], lambda f: got.append(f))

    assert r1 == 1 and r2 == 0
    assert len(got) == 1 and got[0] == frame
    print("[PASS] 半包等待 + 补全")


def test_ota_frame_sizes() -> None:
    """校验各阶段请求帧长度符合协议文档"""
    start = build_ota_start_upgrade_frame(1, 1, 0x10000, 0x89ABCDEF, 0x100000)
    assert len(start) == 6 + 45 + 2
    assert struct.unpack_from("<H", start, 4)[0] == 45

    max_data = ota_max_img_data_per_frame()
    assert max_data == 1072, f"单帧最大 IMG_DATA 应为 1072, 实际 {max_data}"

    write = build_ota_write_data_frame(2, 0, bytes(max_data))
    assert len(write) == 6 + COMM_PROTOCOL_MAX_PAYLOAD_SIZE + 2

    end = build_ota_end_upgrade_frame(3)
    assert struct.unpack_from("<H", end, 4)[0] == 8
    print(f"[PASS] OTA 帧长度 (start={len(start)}, write={len(write)}, end={len(end)}, max_data={max_data})")


def test_ota_parse_start_upgrade() -> None:
    frame = build_ota_start_upgrade_frame(1, 0x01, 0x10000, 0xABCDEF89, 0x100000)
    parsed = parse_frame(frame)
    svc = get_ota_service(parsed)
    assert get_tlv_u32(svc, OTA_TLV_IMG_ID) == 1
    assert get_tlv_u32(svc, OTA_TLV_VERSION) == 0x10000
    assert get_tlv_u32(svc, OTA_TLV_CRC32) == 0xABCDEF89
    assert get_tlv_u32(svc, OTA_TLV_TOTAL_SIZE) == 0x100000
    assert get_tlv(svc, OTA_TLV_TYPE).value == bytes([OTA_OBJECT_IMAGE_RESOURCE])
    assert get_tlv(svc, OTA_TLV_START_UPGRADE_REQUEST).value == b""
    print("[PASS] OTA 开始升级帧解析")


def test_ota_full_flow_small() -> None:
    """完整 OTA：2500 字节，3 块，无 BLE 分片"""
    data = bytes((i * 3 + 7) & 0xFF for i in range(2500))
    run_ota_upgrade(MockOtaDevice(), data, mtu=4096)
    print("[PASS] OTA 全流程 (2500B, 无分片)")


def test_ota_full_flow_mtu20() -> None:
    """完整 OTA + MTU=20 极端分片"""
    data = bytes(range(256)) * 5  # 1280B
    run_ota_upgrade(MockOtaDevice(), data, mtu=20)
    print("[PASS] OTA 全流程 (1280B, MTU=20)")


def test_ota_full_flow_mtu200() -> None:
    """完整 OTA + MTU=200（接近常见 BLE 场景）"""
    data = bytes(0xA5 for _ in range(3072))
    run_ota_upgrade(MockOtaDevice(), data, mtu=200)
    print("[PASS] OTA 全流程 (3072B, MTU=200)")


def test_ota_dirty_data_between_write_blocks() -> None:
    """OTA 多块写入：块与块之间穿插脏数据，复用组包缓冲，升级应仍成功"""
    block1 = bytes((i * 7) & 0xFF for i in range(1024))
    block2 = bytes((i * 3 + 1) & 0xFF for i in range(512))
    file_data = block1 + block2
    device = MockOtaDevice()
    asm = PacketAssemblyBuffer()
    mtu = 64
    seq = 1

    rsp = transceive_via_mtu_persistent(
        asm,
        build_ota_start_upgrade_frame(seq, 1, 0x10000, crc32_resource(file_data), len(file_data)),
        mtu,
        device.handle_request,
    )
    assert get_tlv_u16(get_ota_service(parse_frame(rsp)), PROTOCOL_SERVICE_ERROR_CODE_TLV_ID) == 0

    seq = 2
    rsp = transceive_via_mtu_persistent(
        asm,
        build_ota_write_data_frame(seq, 0, block1),
        mtu,
        device.handle_request,
    )
    assert get_tlv_u16(get_ota_service(parse_frame(rsp)), PROTOCOL_SERVICE_ERROR_CODE_TLV_ID) == 0

    garbage = b"\xDE\xAD\xBE\xEF" + b"\x5C" + bytes(31)  # 块1与块2之间的线路噪声
    seq = 3
    rsp = transceive_via_mtu_persistent(
        asm,
        build_ota_write_data_frame(seq, 1024, block2),
        mtu,
        device.handle_request,
        garbage_before=garbage,
    )
    assert get_tlv_u16(get_ota_service(parse_frame(rsp)), PROTOCOL_SERVICE_ERROR_CODE_TLV_ID) == 0

    seq = 4
    rsp = transceive_via_mtu_persistent(
        asm,
        build_ota_end_upgrade_frame(seq),
        mtu,
        device.handle_request,
    )
    svc = get_ota_service(parse_frame(rsp))
    assert get_tlv_u16(svc, PROTOCOL_SERVICE_ERROR_CODE_TLV_ID) == 0
    assert get_tlv_u16(svc, OTA_TLV_UPGRADE_RESULT) == OTA_STATUS_SUCCESS
    print("[PASS] OTA 块间脏数据 + MTU 分片，全流程仍成功")


def test_ota_crc_fail() -> None:
    """CRC 不匹配应返回 UPGRADE_RESULT=FAIL (0x0001)"""
    data = b"hello ota"
    device = MockOtaDevice()
    wrong_crc = 0x12345678
    rsp = device.handle_request(
        build_ota_start_upgrade_frame(1, 1, 1, wrong_crc, len(data))
    )
    offset = get_tlv_u32(get_ota_service(parse_frame(rsp)), OTA_TLV_IMG_OFFSET_ADDR)
    rsp = device.handle_request(build_ota_write_data_frame(2, offset, data))
    assert get_tlv_u16(get_ota_service(parse_frame(rsp)), PROTOCOL_SERVICE_ERROR_CODE_TLV_ID) == 0
    rsp = device.handle_request(build_ota_end_upgrade_frame(3))
    assert get_tlv_u16(get_ota_service(parse_frame(rsp)), OTA_TLV_UPGRADE_RESULT) == OTA_STATUS_FAIL
    print("[PASS] OTA CRC 校验失败 → UPGRADE_RESULT=0x0001")


def test_ota_type_error() -> None:
    """TYPE != 0x02 时 START 应返回 F000=1"""
    rsp = MockOtaDevice().handle_request(
        build_ota_start_upgrade_frame(1, 1, 1, 0, 100, ota_type=0x01)
    )
    assert get_tlv_u16(get_ota_service(parse_frame(rsp)), PROTOCOL_SERVICE_ERROR_CODE_TLV_ID) == 1
    print("[PASS] OTA TYPE 错误 → F000=1")


def test_ota_write_without_start() -> None:
    """未 START 直接 WRITE 应返回 F000=1"""
    rsp = MockOtaDevice().handle_request(build_ota_write_data_frame(1, 0, b"\x00" * 16))
    assert get_tlv_u16(get_ota_service(parse_frame(rsp)), PROTOCOL_SERVICE_ERROR_CODE_TLV_ID) == 1
    print("[PASS] OTA 未开始升级就写数据 → F000=1")


def test_ota_data_size_mismatch() -> None:
    """数据未传完就 END 应返回 UPGRADE_RESULT=FAIL"""
    data = b"partial"
    device = MockOtaDevice()
    crc32 = crc32_resource(data)
    rsp = device.handle_request(build_ota_start_upgrade_frame(1, 1, 1, crc32, len(data)))
    offset = get_tlv_u32(get_ota_service(parse_frame(rsp)), OTA_TLV_IMG_OFFSET_ADDR)
    # 只写一半
    rsp = device.handle_request(build_ota_write_data_frame(2, offset, data[:4]))
    assert get_tlv_u16(get_ota_service(parse_frame(rsp)), PROTOCOL_SERVICE_ERROR_CODE_TLV_ID) == 0
    rsp = device.handle_request(build_ota_end_upgrade_frame(3))
    assert get_tlv_u16(get_ota_service(parse_frame(rsp)), OTA_TLV_UPGRADE_RESULT) == OTA_STATUS_FAIL
    print("[PASS] OTA 数据大小不足 → UPGRADE_RESULT=0x0001")


def test_crc16_invalid_rejected() -> None:
    """CRC16 错误帧应被组包器丢弃"""
    frame = build_ota_end_upgrade_frame(1)
    bad = bytearray(frame)
    bad[-1] ^= 0xFF
    frames, _ = feed_chunks([bytes(bad)], lambda f: None)
    assert len(frames) == 0
    print("[PASS] CRC16 错误帧被丢弃")


def test_checksum_mismatch_then_recover_mtu200() -> None:
    """CRC 错位后仅滑动 1 字节，后续有效 OTA 写块仍可组包（复现脏数据卡死场景）"""
    block = bytes((i * 7) & 0xFF for i in range(1024))
    good_frame = build_ota_write_data_frame(2, 0xAC, block)
    stale = bytes(900)
    mtu = 200
    chunks = split_chunks(good_frame, mtu)

    asm = PacketAssemblyBuffer()
    asm.write(stale)
    frames: List[bytes] = []
    comm_protocol_assemble_packet(asm, chunks[0], lambda f: frames.append(f))
    for chunk in chunks[1:]:
        comm_protocol_assemble_packet(asm, chunk, lambda f: frames.append(f))

    assert len(frames) == 1
    assert frames[0] == good_frame
    print("[PASS] 缓冲区脏数据 + CRC 错位后可恢复 OTA 写块组包")


def hex_dump(data: bytes, label: str = "") -> None:
    prefix = f"{label}: " if label else ""
    print(prefix + data.hex(" "))


def demo_ota_frames() -> None:
    print("\n── OTA 帧组包演示 ──")
    data = bytes(range(256)) * 4
    crc32 = crc32_resource(data)
    frames = [
        ("开始升级", build_ota_start_upgrade_frame(1, 1, 0x10000, crc32, len(data))),
        ("写数据[0]", build_ota_write_data_frame(2, 0, data[: ota_max_img_data_per_frame()])),
        ("结束升级", build_ota_end_upgrade_frame(3)),
    ]
    for name, frame in frames:
        print(f"\n{name} ({len(frame)} 字节):")
        hex_dump(frame)


TEST_CATALOG = [
    # (函数名, 类别, 说明)
    ("test_single_chunk", "组包", "单包无分片"),
    ("test_mtu_split", "组包", "MTU=200 分片重组"),
    ("test_small_mtu", "组包", "MTU=20 极端分片"),
    ("test_two_frames_back_to_back", "组包", "连续两帧粘包"),
    ("test_invalid_sof_recovery", "组包", "脏数据前缀 SOF 滑动恢复"),
    ("test_dirty_data_between_two_write_frames", "组包", "两帧 WRITE 之间穿插脏数据"),
    ("test_incomplete_then_complete", "组包", "半包等待后补全"),
    ("test_crc16_invalid_rejected", "组包", "CRC16 错误帧丢弃"),
    ("test_checksum_mismatch_then_recover_mtu200", "组包", "CRC 错位后 OTA 写块恢复"),
    ("test_ota_frame_sizes", "OTA", "各阶段帧长度校验"),
    ("test_ota_parse_start_upgrade", "OTA", "开始升级帧 TLV 解析"),
    ("test_ota_full_flow_small", "OTA", "全流程 2500B 无分片"),
    ("test_ota_full_flow_mtu20", "OTA", "全流程 1280B MTU=20"),
    ("test_ota_full_flow_mtu200", "OTA", "全流程 3072B MTU=200"),
    ("test_ota_dirty_data_between_write_blocks", "OTA", "块间脏数据 + 持久组包缓冲"),
    ("test_ota_crc_fail", "OTA异常", "CRC 不匹配 → UPGRADE_RESULT=FAIL"),
    ("test_ota_type_error", "OTA异常", "TYPE 错误 → F000=1"),
    ("test_ota_write_without_start", "OTA异常", "未 START 就 WRITE → F000=1"),
    ("test_ota_data_size_mismatch", "OTA异常", "数据未传完就 END → FAIL"),
]


def print_test_catalog() -> None:
    print("测试用例清单:\n")
    print(f"{'类别':<10} {'用例':<32} 说明")
    print("-" * 70)
    for name, cat, desc in TEST_CATALOG:
        print(f"{cat:<10} {name:<32} {desc}")
    print(f"\n共 {len(TEST_CATALOG)} 项")
    print("\n未覆盖的固件异常（需真机或更完整 Mock）:")
    print("  - 0x000A DATA_BLOCK_TIMEOUT（500ms 块超时）")
    print("  - 0x0009 DATA_WRITE_ERROR（Flash 写入失败）")
    print("  - 断点续传（相同元数据恢复 offset）")
    print("  - NOTIFY 0x000C 主动推送")
    print("  - Payload 超长 (>1088) 拒绝")


def main() -> int:
    global VERBOSE
    parser = argparse.ArgumentParser(description="comm_protocol 组包与 OTA 流程测试")
    parser.add_argument("--demo", action="store_true", help="打印 OTA 帧十六进制")
    parser.add_argument("-v", "--verbose", action="store_true", help="打印 OTA 流程逐步细节")
    parser.add_argument("--list", action="store_true", help="列出全部测试用例与覆盖范围")
    args = parser.parse_args()

    if args.list:
        print_test_catalog()
        return 0

    if args.demo:
        demo_ota_frames()
        return 0

    VERBOSE = args.verbose

    tests = [
        test_single_chunk,
        test_mtu_split,
        test_small_mtu,
        test_two_frames_back_to_back,
        test_invalid_sof_recovery,
        test_dirty_data_between_two_write_frames,
        test_incomplete_then_complete,
        test_crc16_invalid_rejected,
        test_checksum_mismatch_then_recover_mtu200,
        test_ota_frame_sizes,
        test_ota_parse_start_upgrade,
        test_ota_full_flow_small,
        test_ota_full_flow_mtu20,
        test_ota_full_flow_mtu200,
        test_ota_dirty_data_between_write_blocks,
        test_ota_crc_fail,
        test_ota_type_error,
        test_ota_write_without_start,
        test_ota_data_size_mismatch,
    ]

    failed = 0
    for t in tests:
        if VERBOSE:
            print(f"\n>> {t.__name__}: {t.__doc__ or ''}")
        try:
            t()
        except AssertionError as e:
            print(f"[FAIL] {t.__name__}: {e}")
            failed += 1
        except Exception as e:
            print(f"[ERROR] {t.__name__}: {e}")
            failed += 1

    print(f"\n{'=' * 40}")
    if failed:
        print(f"失败 {failed}/{len(tests)}")
        return 1
    print(f"全部通过 {len(tests)}/{len(tests)}  (用 --list 查看清单, -v 查看细节)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
