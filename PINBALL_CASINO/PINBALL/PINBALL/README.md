# Pinball Casino

## Description

Casino-themed pinball game made with Box2D and raylib for the Physics II course (Project 01: Classic Physics).

- **Original game:** {LINK_TO_ORIGINAL_GAME}
- **Source code (GitHub):** {LINK_TO_GITHUB_REPOSITORY}

## Developers

- Lluc Torner Calvo
- Andrea Vasco Peiró
- Alan Del Tio

## How to play

Run `PinballCasino.exe` (keep the `Assets` folder and the DLLs next to it).

Each round has 3 balls. When the last ball is lost the round ends, the score is saved as the previous score
(and as the highest score if it beats it) and a new round can be started.

### Scoring

- Bumper: +100 points
- Target: +50 points
- **Combo:** light up all 3 targets → +1000 points and an **extra ball**
- High Roller lane: +500 points and the ball is launched back up

## Controls

| Key | Action |
| --- | --- |
| Enter | Start game (menu) / play again (game over) |
| M | Back to the main menu (game over screen) |
| Left arrow | Left flipper |
| Right arrow | Right flipper |
| Down arrow | Kicker (hold to pull, release to launch) |
| F1 | Debug mode: draw all physics shapes and enable the mouse joint (drag bodies with the left mouse button) |
| F2 / F3 | Decrease / increase target FPS (10 – 240) |
| F4 / F5 | Decrease / increase gravity |
| F6 / F7 | Decrease / increase bumper bounce (restitution) |
| 1 | Spawn a ball at the mouse position |

## Differences with the original

- {LIST_OF_DIFFERENCES}

## License

This project is licensed under an unmodified MIT license, which is an OSI-certified license that allows static linking with closed source software. Check [LICENSE](LICENSE) for further details.
