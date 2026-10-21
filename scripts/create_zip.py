"""
SleepSense AI - Zip Package Generator
Creates a clean, portable .zip distribution of the entire project
with all dependencies, documentation, firmware, ML pipelines, and backend/frontend code.
"""

import os
import zipfile

def make_project_zip():
    root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    zip_filename = "SleepSense_AI_Project.zip"
    zip_path = os.path.join(root_dir, zip_filename)

    exclude_dirs = {
        "__pycache__",
        "node_modules",
        ".git",
        ".pytest_cache",
        ".vscode",
        ".idea",
        ".system_generated",
        ".venv",
        "env",
        "venv",
        "scratch"
    }
    exclude_extensions = {".pyc", ".pyo", ".tmp", ".log"}

    print(f"Creating clean ZIP distribution: {zip_filename} from {root_dir}...")
    
    file_count = 0
    with zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
        for foldername, subfolders, filenames in os.walk(root_dir):
            rel_folder = os.path.relpath(foldername, root_dir)
            
            # Prune excluded subdirectories
            subfolders[:] = [d for d in subfolders if d not in exclude_dirs and not d.startswith('.')]
            
            # Skip excluded folders
            if rel_folder != ".":
                parts = rel_folder.split(os.sep)
                if any(p in exclude_dirs or p.startswith('.') for p in parts):
                    continue

            for filename in filenames:
                if filename == zip_filename or filename.endswith(".zip"):
                    continue
                _, ext = os.path.splitext(filename)
                if ext in exclude_extensions:
                    continue
                
                filepath = os.path.join(foldername, filename)
                if rel_folder == ".":
                    rel_path = filename
                else:
                    rel_path = os.path.join(rel_folder, filename)
                
                # Standardize forward slashes inside zip archive
                archive_name = "SleepSenseAI/" + rel_path.replace("\\", "/")
                zipf.write(filepath, archive_name)
                file_count += 1

    file_size_mb = os.path.getsize(zip_path) / (1024 * 1024)
    print(f"\n=======================================================")
    print(f" [SUCCESS] Package created successfully!")
    print(f" Filename  : {zip_filename}")
    print(f" Location  : {zip_path}")
    print(f" Total Size: {file_size_mb:.2f} MB")
    print(f" Files     : {file_count} files included")
    print(f"=======================================================\n")

if __name__ == "__main__":
    make_project_zip()
