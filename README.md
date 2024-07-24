# pingee 🌟

Pingee is a fast program that checks the availability of ip addresses and their response times.

[![](https://github-readme-stats.vercel.app/api/pin/?username=shadowvvf&repo=pingee&theme=blue-green)](https://github.com/anuraghazra/github-readme-stats)

## Features

- Check single or multiple network addresses
- Read addresses from a file
- Customize ports to check (default: 80, 443)
- Customizable output format
- Save results to a file
- Multithreaded for improved performance
- Configurable number of scans per address

## Compilation

Compile the program using the following command:

```
g++ -std=c++11 -pthread pingee.cpp -o pingee
```

## Usage

```
./pingee [OPTIONS] ADDRESS [ADDRESS...]
```

### Options

- `-p, --ports PORT1,PORT2,...`: Specify ports to check (default: 80,443)
- `-f, --format FORMAT`: Set custom output format
- `-o, --output FILE`: Save output to a file
- `-t, --threads N`: Set number of threads (default: number of CPU cores)
- `-s, --scans N`: Set number of scans per address (default: 1)

### Output Format

The default output format is:
```
[$status_code$]: $addr$ | $ping$ ms
```

You can customize this using the `-f` option. Available placeholders:
- `$status_code$`: HTTP status code or error code
- `$addr$`: The address being checked
- `$ping$`: Response time in milliseconds

## Examples

1. Check a single address:
   ```
   ./pingee example.com
   ```

2. Check multiple addresses with custom ports:
   ```
   ./pingee example.com google.com -p 80,443,8080
   ```

3. Check addresses from a file with custom output format and save to a file:
   ```
   ./pingee addresses.txt -f "[$status_code$] $addr$ ($ping$ ms)" -o results.txt
   ```

4. Use 8 threads and perform 3 scans per address:
   ```
   ./pingee addresses.txt -t 8 -s 3
   ```
