# Encrypted File Transfer
Brief client-server project that encrypts a file by splitting it into 64-bit blocks using XOR with a key K (64-bit) and sends the encrypted data via socket to the server. The server decrypts, reconstructs the file, and saves it to disk.

## Main Features
- Client: reads file (text or binary), divides into 64-bit blocks, pads the last block with `\0` if necessary, encrypts each block with `Ci = Bi XOR K` using at most `p` threads, sends `[C1C2…Cn, L, K]` to the server, and waits for ack.
- Server: receives message, divides into 64-bit blocks, decrypts with `Bi = Ci XOR K` using `p` threads, reconstructs the file, and saves it with prefix `s`. Sends ack before writing to disk.

## Requirements
- Language: C/C++ (or other with pthreads/sockets support).
- POSIX Environment (socket, signals, threads, file I/O).
- Parameters passed via CLI or environment variables.
- Robust error handling (parameters, system calls).
- Protection against race conditions for shared structures.
- Work blocks and writing not interrupted by signals: SIGINT, SIGALRM, SIGUSR1, SIGUSR2, SIGTERM.

## Client → Server Message Format
- Payload: concatenation of encrypted blocks C = C1 C2 … Cn
- Metadata: original length L (bytes) and key K (unsigned long long int, 64-bit)
- Logical example: [C1C2…Cn, L, K]
- Transport: TCP socket (server listening on specified port)

## Parameters
Client:
- input file: path of the file to encrypt
- K: 64-bit key (unsigned long long int) or represented by a converted string
- p: maximum number of threads for encryption
- server_ip, server_port

Server:
- p: number of threads for decryption
- s: output file name prefix
- l: maximum number of concurrent connections (backlog/thread/process management)

CLI Example:
```
./client -f input.txt -k 0x656d696c69616e6f -p 4 -h 192.168.1.10 -P 9000
./server -p 4 -s output_pref -l 10 -P 9000
```

## Implementation Details
- Block = 64 bits (8 bytes). Read file into buffer, calculate L (bytes), divide into n = ceil(8*L/64) blocks.
- Pad last block with byte `0x00` up to 8 bytes.
- Thread pool client/server side: divide list of block indices among threads to encrypt/decrypt.
- Shared buffer server side: dynamically allocated linked list of blocks; each thread writes its own block in the correct position; synchronize with mutexes and condition variables (avoid race).
- Do not use signals that interrupt critical regions: block signals in threads that encrypt/decrypt and in those that write to disk (pthread_sigmask).
- Handle connection closure: client must handle EOF/close; server sends ack and closes socket server side.

## Error handling
- Validate CLI/ENV parameters (file exists, K valid, p>0, valid port).
- Check returns of socket(), connect(), accept(), read(), write(), send(), recv(), malloc(), pthread_create(), etc. and exit with useful messages.
- Optional timeouts and re-connections.

## Tests and Verification Cases
- Small text file transfer (e.g., 1–3 blocks).
- Binary files (image) to verify integrity after decryption.
- Test with p=1, p>1, and large p (compare output with original).
- Simulate server shutdown during upload and verify client behavior.
- Send incorrect K or incorrect L and verify error handling.
- Concurrency test: many simultaneous clients (≥ l) and verify backlog management.
- Signal test: send SIGINT/SIGTERM during encryption/decryption and verify that critical regions are not interrupted.

## Encryption Example (from spec)
F = "il mio nome e’ legenda" (L = 22 bytes)
K = "emiliano" (binary 64 bit)
- Divide into 3 blocks of 64 bits, third filled with 16 bits of 0.
- C1 = B1 XOR K, etc.
(Use the same process in code to verify with known vector test.)
