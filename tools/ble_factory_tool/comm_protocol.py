"""comm_protocol frame build / parse (aligned with firmware + IMG_RES_OTA_PROTOCOL.md)."""

from __future__ import annotations

import struct
from dataclasses import dataclass, field
from typing import List, Optional, Tuple

COMM_PROTOCOL_SOF = 0x5CC5
COMM_PROTOCOL_HEADER_SIZE = 6
COMM_PROTOCOL_FOOTER_SIZE = 2
COMM_PROTOCOL_SERVICE_HEADER_SIZE = 4
COMM_PROTOCOL_TLV_HEADER_SIZE = 4
COMM_PROTOCOL_MAX_PAYLOAD_SIZE = 4096 + 64  # 4160
COMM_PROTOCOL_OTA_SERVICE = 9
PROTOCOL_SERVICE_ERROR_CODE_TLV_ID = 0xF000

FRAME_TYPE_REQUEST = 0
FRAME_TYPE_RESPONSE = 1
FRAME_TYPE_NOTIFY = 2

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
OTA_BLOCK_SIZE = 4096

OTA_STATUS_SUCCESS = 0x0000
OTA_STATUS_FAIL = 0x0001
OTA_STATUS_DATA_CRC_ERROR = 0x0007
OTA_STATUS_DATA_WRITE_ERROR = 0x0009
OTA_STATUS_DATA_BLOCK_TIMEOUT = 0x000A

MAX_IMG_RES_SIZE = 7 * 1024 * 1024  # 7340032


def crc16_compute(data: bytes, crc: int = 0xFFFF) -> int:
    """Match trunk/root_source/utils/gernal_utils.c crc16_compute()."""
    for b in data:
        crc = ((crc >> 8) & 0xFFFF) | ((crc << 8) & 0xFFFF)
        crc ^= b
        crc ^= (crc & 0xFF) >> 4
        crc ^= ((crc << 8) << 4) & 0xFFFF
        crc ^= (((crc & 0xFF) << 4) << 1) & 0xFFFF
    return crc & 0xFFFF


def build_tlv(tlv_type: int, value: bytes) -> bytes:
    return struct.pack("<HH", tlv_type, len(value)) + value


def build_tlv_u32(tlv_type: int, value: int) -> bytes:
    return build_tlv(tlv_type, struct.pack("<I", value & 0xFFFFFFFF))


def build_tlv_u16(tlv_type: int, value: int) -> bytes:
    return build_tlv(tlv_type, struct.pack("<H", value & 0xFFFF))


def build_tlv_u8(tlv_type: int, value: int) -> bytes:
    return build_tlv(tlv_type, struct.pack("<B", value & 0xFF))


def build_tlv_empty(tlv_type: int) -> bytes:
    return build_tlv(tlv_type, b"")


def build_ota_service(tlvs: bytes) -> bytes:
    return struct.pack("<HH", COMM_PROTOCOL_OTA_SERVICE, len(tlvs)) + tlvs


def build_frame(seq: int, frame_type: int, payload: bytes) -> bytes:
    if len(payload) > COMM_PROTOCOL_MAX_PAYLOAD_SIZE:
        raise ValueError(f"payload 超长: {len(payload)} > {COMM_PROTOCOL_MAX_PAYLOAD_SIZE}")
    header = struct.pack("<HBBH", COMM_PROTOCOL_SOF, seq & 0xFF, frame_type & 0xFF, len(payload))
    body = header + payload
    return body + struct.pack("<H", crc16_compute(body))


def build_ota_request_frame(seq: int, tlvs: bytes) -> bytes:
    return build_frame(seq, FRAME_TYPE_REQUEST, build_ota_service(tlvs))


def build_ota_start_frame(
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


def build_ota_write_frame(seq: int, offset: int, data: bytes) -> bytes:
    tlvs = build_tlv_u32(OTA_TLV_IMG_OFFSET_ADDR, offset) + build_tlv(
        OTA_TLV_WRITE_IMG_DATA, data
    )
    return build_ota_request_frame(seq, tlvs)


def build_ota_end_frame(seq: int, img_id: Optional[int] = None) -> bytes:
    tlvs = b""
    if img_id is not None:
        tlvs += build_tlv_u32(OTA_TLV_IMG_ID, img_id)
    tlvs += build_tlv_empty(OTA_TLV_END_UPGRADE_REQUEST)
    return build_ota_request_frame(seq, tlvs)


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
    total = COMM_PROTOCOL_HEADER_SIZE + payload_len + COMM_PROTOCOL_FOOTER_SIZE
    if len(frame) < total:
        raise ValueError("帧不完整")
    expected_crc = crc16_compute(frame[: COMM_PROTOCOL_HEADER_SIZE + payload_len])
    received_crc = struct.unpack_from("<H", frame, COMM_PROTOCOL_HEADER_SIZE + payload_len)[0]
    if expected_crc != received_crc:
        raise ValueError(f"CRC16 错误: {received_crc:#06x} != {expected_crc:#06x}")

    payload = frame[COMM_PROTOCOL_HEADER_SIZE : COMM_PROTOCOL_HEADER_SIZE + payload_len]
    services: List[ParsedService] = []
    offset = 0
    while offset + COMM_PROTOCOL_SERVICE_HEADER_SIZE <= len(payload):
        svr_id, srv_len = struct.unpack_from("<HH", payload, offset)
        offset += COMM_PROTOCOL_SERVICE_HEADER_SIZE
        if offset + srv_len > len(payload):
            raise ValueError("Service 长度越界")
        tlv_blob = payload[offset : offset + srv_len]
        offset += srv_len
        tlvs: List[ParsedTlv] = []
        tlv_off = 0
        while tlv_off + COMM_PROTOCOL_TLV_HEADER_SIZE <= len(tlv_blob):
            tlv_type, tlv_len = struct.unpack_from("<HH", tlv_blob, tlv_off)
            tlv_off += COMM_PROTOCOL_TLV_HEADER_SIZE
            if tlv_off + tlv_len > len(tlv_blob):
                raise ValueError("TLV 长度越界")
            value = tlv_blob[tlv_off : tlv_off + tlv_len]
            tlv_off += tlv_len
            tlvs.append(ParsedTlv(tlv_type, value))
        services.append(ParsedService(svr_id, tlvs))
    return ParsedFrame(seq, frame_type, payload, services, frame[:total])


def get_ota_service(frame: ParsedFrame) -> Optional[ParsedService]:
    for svc in frame.services:
        if svc.svr_id == COMM_PROTOCOL_OTA_SERVICE:
            return svc
    return None


def get_tlv(service: ParsedService, tlv_type: int) -> Optional[ParsedTlv]:
    for tlv in service.tlvs:
        if tlv.tlv_type == tlv_type:
            return tlv
    return None


def get_tlv_u32(service: ParsedService, tlv_type: int) -> Optional[int]:
    tlv = get_tlv(service, tlv_type)
    if tlv is None or len(tlv.value) != 4:
        return None
    return struct.unpack("<I", tlv.value)[0]


def get_tlv_u16(service: ParsedService, tlv_type: int) -> Optional[int]:
    tlv = get_tlv(service, tlv_type)
    if tlv is None or len(tlv.value) != 2:
        return None
    return struct.unpack("<H", tlv.value)[0]


@dataclass
class FrameAssembler:
    """Accumulate BLE Notify chunks into complete comm_protocol frames."""

    _buf: bytearray = field(default_factory=bytearray)

    def reset(self) -> None:
        self._buf.clear()

    def feed(self, data: bytes) -> List[ParsedFrame]:
        if not data:
            return []
        self._buf.extend(data)
        out: List[ParsedFrame] = []
        while True:
            frame = self._try_extract_one()
            if frame is None:
                break
            out.append(frame)
        # Cap runaway buffer
        if len(self._buf) > COMM_PROTOCOL_MAX_PAYLOAD_SIZE * 4:
            self._buf.clear()
        return out

    def _try_extract_one(self) -> Optional[ParsedFrame]:
        buf = self._buf
        # Find SOF C5 5C
        while True:
            if len(buf) < COMM_PROTOCOL_HEADER_SIZE + COMM_PROTOCOL_FOOTER_SIZE:
                return None
            try:
                idx = buf.index(0xC5)
            except ValueError:
                buf.clear()
                return None
            if idx > 0:
                del buf[:idx]
            if len(buf) < 2:
                return None
            if buf[1] != 0x5C:
                del buf[0]
                continue
            break

        if len(buf) < COMM_PROTOCOL_HEADER_SIZE:
            return None
        payload_len = struct.unpack_from("<H", buf, 4)[0]
        if payload_len > COMM_PROTOCOL_MAX_PAYLOAD_SIZE:
            del buf[0]
            return None
        total = COMM_PROTOCOL_HEADER_SIZE + payload_len + COMM_PROTOCOL_FOOTER_SIZE
        if len(buf) < total:
            return None
        raw = bytes(buf[:total])
        try:
            parsed = parse_frame(raw)
        except ValueError:
            del buf[0]
            return None
        del buf[:total]
        return parsed
