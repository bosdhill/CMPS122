
fp = open("dictionary", "r")
count = 0

passwords = []

for cnt, line in enumerate(fp):
    if line[0] != "#":
        try:
            words = line.split()
            if len(words) < 3:
                print("\ttrying %s" % ''.join([words[0], words[1]]))
                passwords.append(''.join([words[0], words[1]]))
                count += 1
                for word in words:
                    if word.lower() != "city" and word.lower() != "united" and word.lower() != "town":
                        print("\ttrying %s" % word)
                        passwords.append(word)
                        count += 1
                print("")
            else:
                print("\ttrying %s" % ''.join([words[0], words[1], words[2], words[3]]))
                passwords.append(''.join([words[0], words[1], words[2], words[3]]))
                count += 1
                print("\ttrying %s" % ''.join([words[0], words[2], words[3]]))
                passwords.append(''.join([words[0], words[2], words[3]]))
                count += 1
                for word in words:
                    if word != "&":
                        print("\ttrying %s" % word)
                        passwords.append(word)
                        count += 1
                print("")
        except:
            print("\ttrying %s" % line)
            passwords.append(line)
            count += 1

print("trycount = %d" % len(passwords))

print("====")
for passwd in passwords:
    print(passwd.replace('\n', ''))
