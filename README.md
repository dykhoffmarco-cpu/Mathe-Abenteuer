# Mathe-Abenteuer

Ein kleines Mathe-Lernspiel in C++ (SDL2), das mit Emscripten automatisch als Browser-App gebaut wird.

## Was ist enthalten?
- Plus-, Minus- und Endlos-Modus
- Zahlenraum bis 10, 100 oder 1000
- 10, 20, 50 oder 100 Aufgaben in den normalen Modi
- Name, Punkte, richtige/falsche Antworten und Fortschrittsbalken
- Ergebnisbewertung nach der vereinbarten Fehler-Tabelle
- Bestleistung während der aktuellen Sitzung

## Kostenlos im Browser bearbeiten und veröffentlichen

1. Melde dich bei GitHub an und erstelle ein **neues öffentliches Repository**, zum Beispiel `mathe-abenteuer`.
2. Lade die Dateien aus diesem Projekt in das Repository hoch. Die Ordnerstruktur muss so bleiben:
   - `src/main.cpp`
   - `.github/workflows/deploy.yml`
   - `README.md`
3. Öffne im Repository **Settings → Pages**.
4. Stelle bei **Build and deployment → Source** die Option **GitHub Actions** ein.
5. Öffne danach den Reiter **Actions**. Der Workflow „Mathe-Abenteuer bauen und veröffentlichen“ startet automatisch.
6. Wenn der Lauf erfolgreich ist, findest du die veröffentlichte Adresse unter **Settings → Pages**. Sie sieht ungefähr so aus:
   `https://DEIN-BENUTZERNAME.github.io/mathe-abenteuer/`

Wenn du später `src/main.cpp` im GitHub-Browsereditor änderst und speicherst (Commit), wird die Webseite automatisch neu gebaut.

## Wichtig
- Du schreibst den Spielcode in C++. Emscripten erzeugt daraus automatisch HTML, JavaScript-Laufzeitdateien und WebAssembly. Diese erzeugten Dateien musst du nicht selbst programmieren.
- Das ist ein grafisches SDL2-Spiel in einer Browserseite, kein HTML-Formular-Spiel.
- Die Bestleistung wird in dieser ersten Version nur während der geöffneten Sitzung gespeichert. Eine dauerhafte Speicherung können wir später ergänzen.
- Die Schrift ist eine einfache eingebaute Pixel-Schrift, damit keine zusätzliche Schriftdatei benötigt wird.
