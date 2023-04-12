# Read from the file words.txt and output the word frequency list to stdout.
declare -A words;
for word in `cat $1`;
do
    let words[${word}]=(words[${word}]+1);
done
for i in ${!words[*]};
do
    echo ${i} ${words[${i}]};
done | sort -n -k 2 -t ' ' -r