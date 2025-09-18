#!/usr/bin/env python3
#!/usr/bin/env python3

"""Convert design-time images into runtime-ready assets.

This script walks `custom/assets` (or any input directory) for PNG/JPG/WEBP
files, converts them into RGB565 binary blobs, and emits a manifest describing
where the runtime should mount them.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import sys
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Iterable, List

from PIL import Image

SUPPORTED_EXTENSIONS = {".png", ".jpg", ".jpeg", ".webp"}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate runtime asset bundle")
    parser.add_argument("input", type=Path, help="Directory containing source assets")
    parser.add_argument("output", type=Path, help="Directory to write processed assets")
    parser.add_argument(
        "--mount-root",
        default="/spiffs/custom/assets",
        help="Root mount path used by the firmware to locate assets",
    )
    return parser.parse_args()


def slugify(path: Path) -> str:
    # Use the relative path (without extension) as the base id
    parts = path.as_posix()
    parts = re.sub(r"[^0-9a-zA-Z/]+", "_", parts)
    parts = parts.strip("_/")
    parts = parts.replace("/", "_")
    slug = parts.lower()
    return slug or "asset"


@dataclass
class AssetEntry:
    asset_id: str
    rel_path: Path
    width: int
    height: int
    binary_path: Path
    checksum: str

    @property
    def mount_path(self) -> str:
        return self.rel_path.as_posix()


def iter_source_images(root: Path) -> Iterable[Path]:
    for path in sorted(root.rglob("*")):
        if not path.is_file():
            continue
        if path.suffix.lower() not in SUPPORTED_EXTENSIONS:
            continue
        yield path


def to_rgb565(image: Image.Image) -> bytes:
    rgb_image = image.convert("RGB")
    pixels = bytearray()
    for r, g, b in rgb_image.getdata():
        value = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
        pixels.append((value >> 8) & 0xFF)
        pixels.append(value & 0xFF)
    return bytes(pixels)


def process_image(path: Path, root: Path, bin_root: Path) -> AssetEntry:
    rel_path = path.relative_to(root)
    asset_id = slugify(rel_path.with_suffix(""))
    with Image.open(path) as image:
        width, height = image.size
        payload = to_rgb565(image)

    binary_name = f"{asset_id}.bin"
    binary_path = bin_root / binary_name
    binary_path.parent.mkdir(parents=True, exist_ok=True)
    binary_path.write_bytes(payload)

    checksum = hashlib.sha256(payload).hexdigest()
    return AssetEntry(
        asset_id=asset_id,
        rel_path=rel_path,
        width=width,
        height=height,
        binary_path=Path("bin") / binary_name,
        checksum=checksum,
    )


def write_manifest(output_dir: Path, mount_root: str, assets: List[AssetEntry], source_root: Path) -> None:
    manifest_path = output_dir / "manifest.json"
    manifest = {
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "source_root": str(source_root),
        "mount_root": mount_root,
        "assets": [
            {
                "id": entry.asset_id,
                "source": entry.rel_path.as_posix(),
                "binary": entry.binary_path.as_posix(),
                "width": entry.width,
                "height": entry.height,
                "format": "RGB565",
                "checksum": f"sha256:{entry.checksum}",
                "mount_path": f"{mount_root}/{entry.mount_path}",
            }
            for entry in assets
        ],
    }
    output_dir.mkdir(parents=True, exist_ok=True)
    manifest_path.write_text(json.dumps(manifest, indent=2))


def remove_stale_bins(bin_root: Path, assets: List[AssetEntry]) -> None:
    valid = {entry.binary_path for entry in assets}
    if not bin_root.exists():
        return
    for candidate in bin_root.glob("*.bin"):
        rel = Path("bin") / candidate.name
        if rel not in valid:
            candidate.unlink(missing_ok=True)


def main() -> int:
    args = parse_args()
    source_root = args.input.resolve()
    output_root = args.output.resolve()
    bin_root = output_root / "bin"

    if not source_root.exists():
        print(f"Input directory {source_root} does not exist", file=sys.stderr)
        return 1

    assets = [process_image(path, source_root, bin_root) for path in iter_source_images(source_root)]

    remove_stale_bins(bin_root, assets)
    write_manifest(output_root, args.mount_root.rstrip("/"), assets, source_root)

    print(f"Processed {len(assets)} assets into {output_root}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
