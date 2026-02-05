#!/usr/bin/env python3
"""
Génère ik_page.h à partir des fichiers web modifiés
Combine index.html avec les fichiers JS/CSS
"""

import os
import sys

def read_file(path):
    """Lit un fichier et retourne son contenu"""
    try:
        with open(path, 'r', encoding='utf-8') as f:
            return f.read()
    except FileNotFoundError:
        print(f"❌ Fichier non trouvé: {path}")
        sys.exit(1)

def escape_for_cpp(text):
    """Échappe le texte pour C++ raw string"""
    # Remplacer les caractères problématiques
    text = text.replace('\\', '\\\\')
    text = text.replace('"', '\\"')
    return text

def remove_module_tags(js_content):
    """Supprime les imports/exports ES6 pour inline"""
    lines = []
    for line in js_content.split('\n'):
        if line.strip().startswith('import ') or line.strip().startswith('export '):
            lines.append('// ' + line)  # Commenter les imports
        else:
            lines.append(line)
    return '\n'.join(lines)

def generate_ik_page():
    """Génère le fichier ik_page.h"""
    
    # Chemins
    web_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + '/web'
    output_path = os.path.dirname(os.path.abspath(__file__)) + '/../src/webserver/ik_page.h'
    
    print(f"📁 Web directory: {web_dir}")
    
    # Lire les fichiers
    html = read_file(os.path.join(web_dir, 'index.html'))
    css = read_file(os.path.join(web_dir, 'styles.css'))
    js_files = ['config.js', 'ik.js', 'ui.js', 'net.js']
    
    js_content = ''
    for js_file in js_files:
        js_path = os.path.join(web_dir, 'js', js_file)
        js_content += f"\n// ===== {js_file} =====\n"
        js_content += read_file(js_path)
    
    # Intégrer CSS dans HTML
    html = html.replace('</head>', f'<style>\n{css}\n</style>\n</head>')
    
    # Intégrer JS dans HTML (avant </body>)
    js_with_inline = f'<script>\n{js_content}\n</script>'
    html = html.replace('</body>', f'{js_with_inline}\n</body>')
    
    # Générer le fichier C++
    output = f'''#pragma once

/**
 * ik_page.h - Page HTML IK complète avec CSS et JS inline
 * GÉNÉRÉ AUTOMATIQUEMENT - Ne pas éditer directement
 * Utilisez: python3 tools/generate_html_header.py
 */

static const char* HTML_IK_PAGE = R"rawliteral(
{html}
)rawliteral";
'''
    
    # Écrire le fichier
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(output)
    
    print(f"✅ Fichier généré: {output_path}")
    print(f"📊 Taille: {len(output) / 1024:.1f} KB")

if __name__ == '__main__':
    generate_ik_page()
