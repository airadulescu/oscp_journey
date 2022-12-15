for ip in $(seq  1 254)
do 
    host $ip.148.168.192 | grep -v "not found"
done
