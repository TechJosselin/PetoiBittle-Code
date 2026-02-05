#!/usr/bin/env python3
"""
Script pour copier les fichiers web vers le répertoire spiffs
Utilisé par PlatformIO pour préparer les fichiers à être flashés
"""

import os
import shutil
from pathlib import Path

def copy_web_files():
    """Copie les fichiers du dossier web vers spiffs"""
    
    root_dir = Path(__file__).parent
    web_dir = root_dir / "web"
    spiffs_dir = root_dir / "spiffs"
    
    if not web_dir.exists():
        print(f"❌ Web directory not found: {web_dir}")
        return False
    
    # Fichiers à copier
    files_to_copy = [
        ("index.html", "."),
        ("styles.css", "."),
        ("js/config.js", "js"),
        ("js/ik.js", "js"),
        ("js/ui.js", "js"),
        ("js/net.js", "js"),
    ]
    
    for src_path, dst_subdir in files_to_copy:
        src = web_dir / src_path
        dst_dir = spiffs_dir / dst_subdir
        dst = dst_dir / src.name
        
        if not src.exists():
            print(f"⚠️  Fichier non trouvé: {src}")
            continue
        
        dst_dir.mkdir(parents=True, exist_ok=True)
        shutil.copy2(src, dst)
        print(f"✅ Copié: {src_path} -> {dst}")
    
    print(f"✓ Fichiers web copiés vers {spiffs_dir}")
    return True

if __name__ == "__main__":
    copy_web_files()
