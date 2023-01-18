# Summary
# Location
- IP Address: 10.129.6.145
## System Information
- OS: 
- Kernal:
- Hostname: sense
- DomainName: 
- Domain Sid: 
- DNS: 

## Vulnerabilities & Attack Vectors
- Exploit 1
- Exploit 2
- Exploit 3
## Post Exploitation
- Proof
## Credentials
### OS
- uri: https://10.129.6.145/index.php
- username: rohit
- password: pfsense

## System Setup
- ICMP: Enalbed/Disabled
- Firewall: Enabled/Disabled. Will respond to ICMP?
- Essential Services: XXX (TCP XXX)
- Platform:

## Notes

# Service Enumeration
## Port Scan (Nmap)

## Key Infomation
The Nmap version scan identifies an interesting potential entrypoint, in this case, we should focus on th

lighttpd/1.4.35

# Exploitation
## Exploit 1
- Title: pfSense < 2.1.4 - 'status_rrd_graph_img.php' Command Injection
- CVE: 2014-4688
- EBD-ID: 43560
- Link: https://www.exploit-db.com/exploits/43560

need edit the script, when requests do get or post methods, shoud Disable the `verification`.
For example:
exploit_request = client.get(exploit_url, cookies=client.cookies, headers=headers, timeout=5, verify=False)

## Exploit 2
- Title: 
- CVE:
- EBD-ID: 
- Link: 

## Exploit 3
- Title:
- CVE:
- Link:
