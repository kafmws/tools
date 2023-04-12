#!/bin/bash
#github.com
#github.global.ssl.fastly.net
#assets-cdn.github.com
# TEST TEST TEST TEST TEST TEST TEST
# hosts=result
# domians=('assets-cdn.github.com')
# links=('https://github.com.ipaddress.com/assets-cdn.github.com')
# TEST TEST TEST TEST TEST TEST TEST
hosts=/mnt/c/Windows/System32/drivers/etc/hosts #win
domains=('github.com' 'github.global.ssl.fastly.net' 'assets-cdn.github.com')
links=('https://github.com.ipaddress.com/' 'https://fastly.net.ipaddress.com/github.global.ssl.fastly.net' 'https://github.com.ipaddress.com/assets-cdn.github.com')
prefix="<th>IPv4 Addresses</th><td><ul class=\"comma-separated\"><li>"
let prefixLen=(${#prefix}+1)
#query ip and add to hosts
echo -e "\n#hosts-ex.sh add Start\n" >>$hosts
for (( i=0; i<${#domains[@]}; i=i+1));
do
    ips=(`curl  ${links[$i]} \
        | grep -Eo "${prefix}([0-9]{1,2}|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.([0-9]{1,2}|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.([0-9]{1,2}|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.([0-9]{1,2}|1[0-9][0-9]|2[0-4][0-9]|25[0-5])"\
        | cut -c $prefixLen-`);
    #ips may contains few ip
    for ip in ${ips[*]};
    do echo -e "$ip\t\t${domains[$i]}\n" >>$hosts; echo -e "$ip\t\t${domains[$i]}\n"; done
done
echo -e "#hosts-ex.sh add End\n" >>$hosts