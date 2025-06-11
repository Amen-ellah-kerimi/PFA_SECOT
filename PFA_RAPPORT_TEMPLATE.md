---
title: "Système IoT Sécurisé et Intégré (Station Météo + Feux de Circulation)"
author: "Amen Ellah Kerimi"
supervisor: "Dr. XYZ"
university: "Université de Carthage"
faculty: "Faculté des Sciences de Bizerte"
department: "Département de Génie Informatique"
year: "Année universitaire 2024–2025"
logo: fsb_logo.png
geometry: margin=2.5cm
fontsize: 12pt
lang: fr
---

\begin{titlepage}
    \centering
    {\LARGE \textbf{Université de Carthage}}\\[0.5cm]
    {\Large Faculté des Sciences de Bizerte}\\[0.5cm]
    {\large Département de Génie Informatique}\\[1.5cm]
    \vspace{1cm}
    {\Huge \textbf{Système IoT Sécurisé et Intégré}}\\[0.5cm]
    {\Large (Station Météo + Feux de Circulation)}\\[2cm]
    \vspace{1cm}
    \includegraphics[width=0.25\textwidth]{fsb_logo.png}\\[1cm]
    \vspace{1cm}
    \begin{tabular}{rl}
        \textbf{Auteur :} & Amen Ellah Kerimi \\
        \textbf{Encadrant :} & Dr. XYZ \\
        \textbf{Année universitaire :} & 2024--2025 \\
    \end{tabular}
    \vfill
\end{titlepage}

\newpage

# Table des matières

\tableofcontents

\newpage

# Introduction


# Problématique


# Cahier des charges


# Étude de l’existant


# Choix techniques


# Architecture du système


# Implémentation


# Résultats


# Perspectives


# Conclusion


# Bibliographie


---

## \textbf{Instructions d’export PDF}

Pour générer un PDF avec Pandoc et le template Eisvogel :

```powershell
pandoc PFA_RAPPORT_TEMPLATE.md -o PFA_RAPPORT.pdf --from markdown --template eisvogel --listings --pdf-engine=xelatex
```

- Pour le mode sombre (si supporté) :

```powershell
pandoc PFA_RAPPORT_TEMPLATE.md -o PFA_RAPPORT_dark.pdf --from markdown --template eisvogel --listings --pdf-engine=xelatex -V colorlinks=true -V linkcolor=cyan -V urlcolor=cyan -V toccolor=cyan -V theme=dark
```

- La numérotation des pages commence après la page de garde automatiquement avec Eisvogel.

---

## \textbf{Exemple de code Arduino}

```cpp
// Contrôle d’un feu de circulation
void loop() {
  digitalWrite(RED, HIGH);
  delay(5000);
  digitalWrite(RED, LOW);
  digitalWrite(GREEN, HIGH);
  delay(5000);
  digitalWrite(GREEN, LOW);
  digitalWrite(YELLOW, HIGH);
  delay(2000);
  digitalWrite(YELLOW, LOW);
}
```

## \textbf{Exemple de code Python}

```python
import paho.mqtt.client as mqtt
client = mqtt.Client()
client.connect('localhost', 1883)
client.publish('home/trafficlight', 'GREEN')
```

## \textbf{Exemple de formule LaTeX}

$$
I = \frac{V}{R}
$$

---

> \textit{Ce document est un modèle structuré pour la rédaction du rapport de PFA, prêt à être complété et exporté en PDF académique.}
