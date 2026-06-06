import os
from PIL import Image

ASSET_SIZES = {
    "StoreLogo.png": (50, 50),
    "Square150x150Logo.png": (150, 150),
    "Square44x44Logo.png": (44, 44),
    "Wide310x150Logo.png": (310, 150),
    "SmallTile.png": (71, 71),
    "SplashScreen.png": (620, 300),
}

def generate_assets(src_path: str, out_dir: str):
    if not os.path.exists(src_path):
        print(f"Error: {src_path} not found.")
        return False

    img = Image.open(src_path)
    os.makedirs(out_dir, exist_ok=True)

    for name, size in ASSET_SIZES.items():
        dst = os.path.join(out_dir, name)
        resized = img.resize(size, Image.LANCZOS)
        resized.save(dst)
        print(f"  Created {dst} ({size[0]}x{size[1]})")

    print("All store assets generated successfully.")
    return True

if __name__ == "__main__":
    import sys
    src = sys.argv[1] if len(sys.argv) > 1 else "logo.png"
    out = sys.argv[2] if len(sys.argv) > 2 else "Assets"
    print(f"Generating store assets from {src} -> {out}")
    generate_assets(src, out)
