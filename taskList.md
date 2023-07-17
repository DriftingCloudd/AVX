ash -c exit (usertrap)
sh -c exit (usertrap)
cal (openat error)
df (usertrap)
dmesg (syscall 116)
du (syscall 79)
which ls (syscall 48)
uptime (syscall 179)
printf "abc\n" (remove返回了-1)
ps (usertrap)
pwd (usertrap)
free (29,66,openat)
hwclock (openat)
kill 10 (syscall 129)
ls (usertrap)
touch test.txt (88)
echo "hello world" > test.txt
cat test.txt
cut -c 3 test.txt
od test.txt
head test.txt
tail test.txt 
hexdump -C test.txt 
md5sum test.txt
echo "ccccccc" >> test.txt
echo "bbbbbbb" >> test.txt
echo "aaaaaaa" >> test.txt
echo "2222222" >> test.txt
echo "1111111" >> test.txt
echo "bbbbbbb" >> test.txt
sort test.txt | ./busybox uniq
stat test.txt
strings test.txt 
wc test.txt
[ -f test.txt ]
more test.txt
rm test.txt
mkdir test_dir
mv test_dir test
rmdir test
grep hello busybox_cmd.txt
cp busybox_cmd.txt busybox_cmd.bak
rm busybox_cmd.bak
find -name "busybox_cmd.txt"
