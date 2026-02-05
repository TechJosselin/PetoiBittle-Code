#!/usr/bin/env python3
"""
Script pour générer dynamiquement ik_page.h à partir de web/index.html
Exécuté en tant que hook AVANT le build pour mettre à jour la page HTML compilée
"""

from pathlib import Path
import sys

def generate_ik_page_header():
    """Génère ik_page.h à partir de web/index.html"""
    
    # Déterminer les chemins
    if len(sys.argv) > 1:
        project_dir = Path(sys.argv[-1])
    else:
        project_dir = Path(__file__).parent.parent
    
    web_index = project_dir / "web" / "index.html"
    output_header = project_dir / "include" / "ik_page.h"
    
    print("\n" + "="*70)
    print("📝 GENERATION: ik_page.h <- web/index.html")
    print("="*70)
    
    # Vérifier que web/index.html existe
    if not web_index.exists():
        print(f"❌ Erreur: {web_index} n'existe pas")
        return False
    
    try:
        # Lire web/index.html
        with open(web_index, 'r', encoding='utf-8') as f:
            html_content = f.read()
        
        # Créer le header C++
        header_content = '''#pragma once

/**
 * ik_page.h - Page HTML IK générée automatiquement depuis web/index.html
 * ATTENTION: Ce fichier est généré automatiquement par tools/generate_ik_page.py
 * Ne pas éditer directement - modifiez web/index.html à la place !
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
        header_size = len(header_content)
        print(f"  ✅ web/index.html : {html_size} bytes")
        print(f"  ✅ include/ik_page.h generé : {header_size} bytes")
        print("="*70 + "\n")
        
        return True
        
    except Exception as e:
        print(f"❌ Erreur génération: {e}")
        print("="*70 + "\n")
        return False

# Hook PlatformIO
def pre_build(env):
    """Appelé par PlatformIO avant le build"""
    success = generate_ik_page_header()
    if not success:
        print("⚠️  Avertissement: ik_page.h n'a pas pu être mis à jour")

# Exécution directe
if __name__ == "__main__":
    success = generate_ik_page_header()
    sys.exit(0 if success else 1)
