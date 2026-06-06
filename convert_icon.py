import os
try:
    # pyrefly: ignore [missing-import]
    from PIL import Image
except ImportError:
    print("Pillow library is required. Install it using: pip install Pillow")
    exit(1)

def convert_png_to_ico(png_path, ico_path):
    if not os.path.exists(png_path):
        print(f"Error: {png_path} not found.")
        return
        
    try:
        img = Image.open(png_path)
        # Create an ICO file with multiple sizes to ensure maximum compatibility
        icon_sizes = [(256, 256), (128, 128), (64, 64), (48, 48), (32, 32), (16, 16)]
        img.save(ico_path, format='ICO', sizes=icon_sizes)
        print(f"Successfully created a valid multi-resolution ICO file at: {ico_path}")
    except Exception as e:
        print(f"Error converting image: {e}")

if __name__ == "__main__":
    # You can change 'logo.png' to your actual source PNG file
    source_png = "logo.png"
    target_ico = "setup_icon.ico"
    
    print("Generating proper ICO file...")
    convert_png_to_ico(source_png, target_ico)
