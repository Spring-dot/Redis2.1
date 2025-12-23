
for i in {1..1000}; do 
    echo "hi" | nc -q 0 localhost 6379; 
done
