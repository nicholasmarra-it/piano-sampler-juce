# Piano Sampler – plugin audio in C++ con JUCE

Sampler di pianoforte in formato **VST3** e **applicazione standalone**, scritto in **C++** con il framework **JUCE 8**. Progetto di gruppo (4 persone) del corso di Sound Design, Università Politecnica delle Marche.

> **English:** Piano sampler audio plugin (VST3 + standalone) written in C++ with JUCE 8: sample loading with waveform display, loop recording and overdub, ADSR envelopes, resonant filter with its own envelope, LFO, saturation, lo-fi and creative effects, and a preset system. University group project (4 people).

## Funzionalità

- **Campioni:** caricamento di file audio con visualizzazione della forma d'onda
- **Loop:** punti di inizio e fine regolabili, riproduzione al contrario, registrazione di loop con overdub
- **Modalità di esecuzione:** polifonica, mono legato e mono retrigger, con funzione Hold
- **Inviluppi:** ADSR sul volume e ADSR dedicato al filtro
- **Filtro:** passa-basso, passa-alto e passa-banda con risonanza
- **LFO:** sinusoide, quadra, triangolo, dente di sega e random, assegnabile a filtro, volume e intonazione
- **Saturazione** con controllo di drive e mix
- **Effetti lo-fi:** bit crush, riduzione della frequenza di campionamento, rumore
- **Effetti creativi:** stereo width, autopan, stutter, riproduzione casuale con variazione di intonazione
- **Preset** salvabili e richiamabili
- Si suona via **MIDI** o direttamente dalla **tastiera del PC**

## Struttura del progetto

| Percorso | Contenuto |
|---|---|
| `Source/PluginProcessor.*` | Motore audio: sintetizzatore, parametri, effetti, loop e preset |
| `Source/PluginEditor.*` | Interfaccia grafica del plugin |
| `Source/SamplerSound.*`, `Source/SamplerVoice.*` | Gestione dei campioni e delle voci |
| `Source/LowpassHighpassFilter.h` | Filtro passa-basso / passa-alto / passa-banda |
| `Source/WaveformDisplay.*` | Visualizzazione della forma d'onda |
| `Source/TestPresetLoading.h` | Test del caricamento dei preset |
| `ProgettoSound.jucer` | Progetto del Projucer |

## Compilazione

Requisiti: JUCE 8 (Projucer) e Visual Studio 2022 su Windows.

1. Apri `ProgettoSound.jucer` con il Projucer.
2. Controlla che il percorso dei moduli JUCE sia corretto (*Global Paths*).
3. Clicca **Save and Open in IDE** e compila la configurazione **Release**: otterrai il plugin VST3 e l'applicazione standalone.

## Licenza

Codice distribuito con licenza **GNU AGPLv3**, in linea con la licenza open source di JUCE 8.
