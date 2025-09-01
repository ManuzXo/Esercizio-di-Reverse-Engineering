# Un'esempio di reverse engineering
Voglio condividere con voi un'esempio di [reverse engineering](https://it.wikipedia.org/wiki/Reverse_engineering), un'esercizio che ho ideato, dove il programma **crackme.exe** ti chiede una key da inserire, se la key inserita è sbagliata, darà errore.  
Per questo ho voluto provare ad implementare una metodologia di *'Hacking'* se vogliamo, di base è sfruttare il [codice macchina](https://it.wikipedia.org/wiki/Linguaggio_assembly) del sorgente [**crackme.exe**](https://github.com/ManuzXo/Esercizio-di-Reverse-Engineering/blob/main/crackme.exe).
## 🔧 Tecnologia Usata
### Linguaggio Usato
Per programmare l'eseguibile [**crackme.exe**](https://github.com/ManuzXo/Esercizio-di-Reverse-Engineering/blob/main/crackme.exe) & [**cheat.exe**](https://github.com/ManuzXo/Esercizio-di-Reverse-Engineering/blob/main/cheat.exe) ho usato il linguaggio [**C**](https://it.wikipedia.org/wiki/C_(linguaggio_di_programmazione)).  
### Disassembler
Per [disassemblare](https://it.wikipedia.org/wiki/Disassembler) il programma **crackme.exe** ho usato il software [IDA HexRays](https://hex-rays.com/) (Versione Gratuita)  
### Compilazione
Per compilare i programmi (compilati per 32 bit), ho usato [GCC](https://gcc.gnu.org/) per ambiente Windows
## ⚠ Premessa
- In questa spiegazione tratterò di temi come lo [stack](https://en.wikipedia.org/wiki/Stack_(abstract_data_type)) e l'[heap](https://it.wikipedia.org/wiki/Allocazione_dinamica_della_memoria), quindi non spiegherò tutto nei mini dettagli, anche perchè dovrei scrivere un papiro 😢.  
- Per facilitare anche il disassemblaggio, ho compilato una sola volta l'eseguibile [**crackme.exe**](https://github.com/ManuzXo/Esercizio-di-Reverse-Engineering/blob/main/crackme.exe). Il motivo è per gli [indirizzi di memoria](https://en.wikipedia.org/wiki/Memory_address), se ad ogni test facessimo una nuova compilazione, tutti gli **indirizzi di memoria** verrebbero cambiati nel file eseguibile, quindi per facilitare anche lo sviluppo del [**cheat.c**](#cheatc-x86), gli **indirizzi di memoria** che vedrete saranno collegati direttamente all'eseguibile **crack.exe** già compilato.  
Se volete provare anche ad implementare una logica dinamica ad ogni compilazione nuova dell'eseguibile vi consiglio questo [link](https://github.com/GH-Rake/PatternScan)
## [crackme.c](https://github.com/ManuzXo/Esercizio-di-Reverse-Engineering/blob/main/src/crackme.c) (x86)
Questo è il programma che farà da cavia per questo esercizio.  
Al suo interno ho scritto un piccolo key generator dinamico, quindi ad ogni avvio del programma, la key sarà sempre con un valore diverso. 
![crackme_cmd](https://github.com/user-attachments/assets/19e748de-63db-43c4-8f2c-f970de569660)
## 👩‍💻 Steps per il Reversing
Bene ora il nostro scopo è quello di capire come il codice macchina è strutturato, e una volta capito si può pensare a come sviluppare un programma che legge la chiave senza il suo consenso 😎.  
Ora che abbiamo una vista generale di come l'eseguibile funziona, sappiamo che è un'applicazione ConsoleCommand/Terminale e che stampa a schermo il messaggio [**Insert the key:**](https://github.com/ManuzXo/Esercizio-di-Reverse-Engineering/blob/main/src/crackme.c#L35).  
Quindi grazie a questa piccola informazione, possiamo aprire il disassemblatore **IDA** e far analizzare il file **crackme.exe**.
- Finita l'analisi, possiamo provare a cercare la stringa **Insert the Key:**
![ida_search_string](https://github.com/user-attachments/assets/3342febd-a6cd-4bc9-82f8-1f6ffcb9d02a)  
- Una volta seguito il video, sarete dentro la logica principale per l'inserimento della key, con i suoi controlli ecc. Ora proveremo a seguire come il nostro input viene salvato e usato.
  <img width="1467" height="767" alt="analyze_asm" src="https://github.com/user-attachments/assets/e0fe416f-5b1f-4db0-95a5-cbbfc52dcf4a" />
- Perfetto ora che abbiamo visto come il nostro input viene usato nello stack, possiamo ora vedere anche come viene salvata e utilizzata la key.
  <img width="1467" height="767" alt="analyze_asm" src="https://github.com/user-attachments/assets/ec581a25-48df-4fbf-839c-6262fc25b5d2" />

  ### Curiosità variabili IDA
  Voglio precisare giusto una cosa se magari non è chiara, le variabili che si vedono tipo **Block**, è il programma **IDA** che prova a ricostruire la struttura dello [stack frame](https://it.wikipedia.org/wiki/Stack_frame), ma in realtà        bisogna fare il calcolo effettivo, esempio:
  <img width="966" height="665" alt="math_offset" src="https://github.com/user-attachments/assets/68563e72-06a8-4dae-9613-b11d45252149" />

## ℹ Dati acquisiti 
Ora che sappiamo dove vengono salvati i dati nello *stack frame*, abbiamo varie possibilità:  
1. <ins>EASY WAY</ins>  
    Fare una patch al programma e saltare nel punto dove stampa **Key is Correct**  
    <img width="450" height="133" alt="block_key_correct" src="https://github.com/user-attachments/assets/18e4c1e6-8773-4235-b003-3f52c311bf84" />  
    
2. <ins>COMPLEX WAY</ins>  
    Possiamo andare a programmare un piccolo [shellcode/payload](https://en.wikipedia.org/wiki/Shellcode).  
    
La prima è troppo da skiddy, quindi si andrà per la **2**🤣.
## [cheat.c](https://github.com/ManuzXo/Esercizio-di-Reverse-Engineering/blob/main/src/cheat.c) (x86)
Ora sappiamo dove nel programma stia la logica base di generazione **key** e quale registri usare.  
La mia 💡 era di stampare la chiave prima che il programma ti chieda **Insert the Key**, quindi un'esempio di shell code da injectare all'interno della funzione sarebbe:  
```asm
   push [esp+60h+Block] ; puntatore dove è la chiave
   push offset strFormat ; qui sarà la stringa del tipo "%s"
   call printf
```
--------------------------
### ❓ Come e Dove
Domanda spontanea, come e dove lo inserisci nel programma?  
Possiamo benissimo decidere di applicare un [Hook](https://en.wikipedia.org/wiki/Hooking), tramite l'istruzione [jmp](https://en.wikipedia.org/wiki/JMP_(x86_instruction)) al nostro payload, così che il programma eseguirà le nostre istruzioni. 
<img width="1546" height="477" alt="payload_insert" src="https://github.com/user-attachments/assets/96a50888-1186-4a76-a8d1-ffdafe42bd56" />
### 🔧 Implementazione
Per l'implementazione non mi dilungo nello spiegare ogni singola riga di codice, ma darò un'overview di quello che ho usato.  
Di base, per implementare il payload, abbiamo bisogno di [librerie di sistema](https://it.wikipedia.org/wiki/Libreria_standard_del_C#:~:text=La%20libreria%20standard%20del%20C,della%20libreria%20ad%20esse%20associate.) e degli indirizzi di memoria del programma.  

#### 📚 Librerie di Sistema (più rilevanti)
- [CreateProcessW](https://learn.microsoft.com/it-it/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessw) 🡆 Avvia l'eseguibile **crackme.exe** con la modalità [CREATE_SUSPEND](https://learn.microsoft.com/en-us/windows/win32/procthread/process-creation-flags#:~:text=Virtual%20DOS%20Machine.-,CREATE_SUSPENDED,-0x00000004) in modo che il programma sia in uno stato di attesa
- [VirtualAllocEx](https://learn.microsoft.com/it-it/windows/win32/api/memoryapi/nf-memoryapi-virtualallocex) 🡆 Usata sia per allocare la stringa che stamperà la [key](https://github.com/ManuzXo/Esercizio-di-Reverse-Engineering/blob/main/src/cheat.c#L22) e per [allocare il payload](https://github.com/ManuzXo/Esercizio-di-Reverse-Engineering/blob/main/src/cheat.c#L71)
- [WriteProcessMemory](https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-writeprocessmemory) 🡆 Usata per fare [l'hook del payload](https://github.com/ManuzXo/Esercizio-di-Reverse-Engineering/blob/main/src/cheat.c#L86) e per scrivere i [bytes/opcode del payload](https://github.com/ManuzXo/Esercizio-di-Reverse-Engineering/blob/main/src/cheat.c#L28)

#### 🧩 Indirizzi di Memoria
Ci servono soltanto due indirizzi di memoria del programma: **0x40150C** e **0x401510**
<img width="1250" height="477" alt="offset_memory" src="https://github.com/user-attachments/assets/3b99ed6c-017b-4755-af1f-4d5122db07e1" />
## 🎯 Risultato Finale
![risulato_finale](https://github.com/user-attachments/assets/c48fa2ca-59ec-4c8d-9ef2-a71de8f45dab)












