#!/usr/bin/env python3
"""
PlatformIO pre-build hook pour préparer les fichiers
1. Génère ik_page.h à partir de data/index.html (fallback compilé)
2. Les fichiers data sont intégrés à littleFS via platformio.ini (board_build.embed_files)
"""

import os
import sys
import shutil
import subprocess
from pathlib import Path

def generate_ik_page_header():
    """Génère ik_page.h à partir de data/index.html"""
    
    script_dir = Path(__file__).parent
    data_index = script_dir / "data" / "index.html"
    output_header = script_dir / "include" / "ik_page.h"
    
    print("\n" + "="*70)
    print("📝 GENERATION: ik_page.h <- data/index.html (fallback compilé)")
    print("="*70)
    
    if not data_index.exists():
        print(f"❌ Erreur: {data_index} n'existe pas")
        return False
    
    try:
        # Lire data/index.html
        with open(data_index, 'r', encoding='utf-8') as f:
            html_content = f.read()
        
        # Créer le header C++
        header_content = '''#pragma once

/**
 * ik_page.h - Page HTML IK générée automatiquement depuis data/index.html
 * Fallback compilé en cas de problème avec littleFS
 * ATTENTION: Ce fichier est généré automatiquement par pre_build_hook.py
 * Ne pas éditer directement - modifiez data/index.html à la place !
 */

static const char* HTML_IK_PAGE = R"rawliteral(
'''
        
        # Ajouter le contenu HTML
        header_content += html_content
        
        # Fermer le raw string
        header_content += '''
)rawliteral";
'''
        
        # Écrire ik_page.h
        with open(output_header, 'w', encoding='utf-8') as f:
            f.write(header_content)
        
        # Statistiques
        html_size = len(html_content)
        print(f"  ✅ data/index.html : {html_size} bytes")
        print(f"  ✅ include/ik_page.h généré")
        return True
        
    except Exception as e:
        print(f"❌ Erreur génération: {e}")
        return False

# Hook PlatformIO appelé AVANT le build
def pre_build(env):
    """
    Hook PlatformIO exécuté avant la compilation
    Génère ik_page.h à partir de data/index.html (fallback compilé)
    
    Les fichiers data sont directement intégrés à littleFS par:
    board_build.embed_files = data/index.html, data/styles.css, etc.
    """
    
    # Générer ik_page.h (fallback compilé)
    if not generate_ik_page_header():
        print("⚠️  Avertissement: ik_page.h n'a pas pu être mis à jour")
    
    print("✓ Préparation du build terminée (littleFS)")
    print("="*70 + "\n")

# Exécution directe / manuelle
if __name__ == "__main__":
    print("\n🔨 Exécution manuelle du hook pre_build...\n")
    generate_ik_page_header()

