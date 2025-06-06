# ESP-Doorlock

Hier ist der Sachverhalt aus dem handschriftlichen Diagramm digital und strukturiert dargestellt:

---

### 🔐 **Hybrid-Verschlüsselung mit RSA und AES**

#### **Ablaufdiagramm**

```mermaid
sequenceDiagram
    participant C as Client
    participant S as Server

    Note over C,S: Hybridverschlüsselung (RSA + AES)

    Note over C:  RSA-Key aufgefüllt mit 0 als String

    C->>S: Activate UUID_KEY_CHARACTERISTIC "df5ba2aa-c90c-4c90-8c5f-059f62ff51a1" notify

    C->>+S: RSA-pub Schlüssel senden

    Note over S: AES Session-Key generieren<br/>AES-GCM Modus mit SHA-256<br/>mit preshared IV (Initialisierungsvektor)
    Note over S: AES-Key mit public key nach (RSA PKCS1 v2.1 OAEP) verschlüssen

    S->>-C: Notify change Encrypted-AES-Key



    Note over C: Nachricht mit AES-GCM<br/>verschlüsseln (16 Bytes Daten +<br/>16 Bytes Authentifizierungstag)

    C->>+S: Verschlüsselte Nachricht senden +  Authentifizierungstag
```

---

### ✳️ Details zur Implementierung

#### **RSA**

- **Modus**: `RSA PKCS#1 v2.1 (OAEP)`
- **Hash-Funktion**: `SHA-256`
- **Hinweis**: **NICHT** `v1.5` verwenden → **Bleichenbacher-Angriff** möglich

#### **AES**

- **Modus**: `AES-GCM`
- **IV (Initialisierungsvektor)**:

  - Wird **einmalig zufällig** generiert und dokumentiert

- **Tag (Authentifizierungs-Tag)**:

  - Wird **mit dem verschlüsselten Text** übergeben
  - Format: `16 Byte verschlüsselte Daten + 16 Byte Tag`

```c++
uint8_t m_aes_iv[12] = {0x46, 0x61, 0x63, 0x68, 0x73, 0x63,
                        0x68, 0x61, 0x66, 0x74, 0x45, 0x54};
```

---

Möchtest du daraus auch eine Grafik im Stil eines Blockdiagramms, z. B. als SVG oder PNG?
