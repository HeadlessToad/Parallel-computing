import subprocess
import sys

def install(package):
    subprocess.check_call([sys.executable, "-m", "pip", "install", package])

try:
    import pypdf
except ImportError:
    install('pypdf')
    import pypdf

from pypdf import PdfReader

import argparse

def extract_pdf_text(filepath, output_path):
    try:
        reader = PdfReader(filepath)
        text = ""
        for i, page in enumerate(reader.pages):
            text += f"--- Page {i+1} ---\n"
            text += page.extract_text() + "\n"
        
        with open(output_path, "w", encoding="utf-8") as f:
            f.write(text)
        print(f"Successfully extracted text to {output_path}")
    except Exception as e:
        print(f"Error reading PDF: {e}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Extract text from PDF")
    parser.add_argument("input_pdf", help="Path to input PDF file")
    parser.add_argument("output_txt", help="Path to output text file")
    args = parser.parse_args()
    
    extract_pdf_text(args.input_pdf, args.output_txt)
