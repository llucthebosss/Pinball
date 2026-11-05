# Pinball Casino

## Descripció

Joc de pinball amb temàtica de casino fet amb Box2D i raylib per a l'assignatura de Física II (Projecte 01: Classic Physics).

- **Codi font (GitHub):** https://github.com/llucthebosss/Pinball

## Desenvolupadors

- Lluc Torner Calvo
- Andrea Vasco Peiró
- Alan Del Tio

## Com jugar

Executa `PinballCasino.exe` (mantén la carpeta `Assets` i les DLL al costat de l'executable).

Cada ronda té 3 boles. Quan es perd l'última bola, la ronda s'acaba: la puntuació es guarda com a puntuació anterior
(i com a millor puntuació si la supera) i es pot començar una ronda nova.

### Puntuació

- Bumper: +100 punts
- Diana: +50 punts
- **Combo:** encendre les 3 dianes → +1000 punts i una **bola extra**
- Carril High Roller: +500 punts i la bola es torna a llançar cap amunt

## Controls

| Tecla | Acció |
| --- | --- |
| Enter | Començar la partida (menú) / tornar a jugar (game over) |
| M | Tornar al menú principal (pantalla de game over) |
| Fletxa esquerra | Flipper esquerre |
| Fletxa dreta | Flipper dret |
| Fletxa avall | Kicker (mantén per estirar, deixa anar per llançar) |
| F1 | Mode debug: dibuixa totes les formes físiques i activa el mouse joint (arrossega cossos amb el botó esquerre del ratolí) |
| F2 / F3 | Baixar / pujar els FPS objectiu (10 – 240) |
| F4 / F5 | Baixar / pujar la gravetat |
| F6 / F7 | Baixar / pujar el rebot dels bumpers (restitució) |
| 1 | Crear una bola a la posició del ratolí |

## Diferències amb l'original

- {LLISTA_DE_DIFERÈNCIES}

## Llicència

Aquest projecte està sota una llicència MIT sense modificar, una llicència certificada per l'OSI que permet l'enllaçat estàtic amb programari de codi tancat. Consulta [LICENSE](LICENSE) per a més detalls.
