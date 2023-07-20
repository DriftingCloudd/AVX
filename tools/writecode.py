i = 0
while(i < 55):
    print("\tli a7, SYS_fork\n\tecall\n\tbne a0, zero, after_"+str(i)+"\n\tla a0, busybox"+"\n\tla a1, argv_busybox_"+str(i)+"\n\tli a7, SYS_exec\n\tecall\nafter_"+str(i)+":")
    print("\tli a0, 0")
    print("\tli a7, SYS_wait")
    print("\tecall")
    print("")
    i += 1


with open("../sdcard/busybox_cmd.txt", 'r') as file:
    i = 0
    for line in file:
        j = 0
        tokens = []
        in_quote = False
        current_token = ""

        for char in line.strip():
            if char == '"':
                in_quote = not in_quote
                current_token += char
            elif char == ' ' and not in_quote:
                tokens.append(current_token)
                current_token = ""
            else:
                current_token += char

        if current_token:
            tokens.append(current_token)

        for token in tokens:
            print("param" + str(i) + str(j) +":")
            if token[0] != '"':
                print('\t.string "'+ token + '\\0"')
            else:
                tmp = token[0:len(token)-1]
                tmp += '\\0"'
                print('\t.string ' + tmp)
            j += 1
        
        print("argv_busybox_"+str(i)+":")
        print("\t.dword busybox")
        for k in range(j):
            print("\t.dword param"+str(i)+str(k))
        print("\t.dword 0")
        i += 1


