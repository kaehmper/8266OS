with open("NumberStationOS/NumberStationOS.ino", "r") as f:
    content = f.read()

# Fix the literal null character warning
content = content.replace("!= '\\0'", "!= 0")
content = content.replace("!= '\\0'", "!= 0")

# Fix the extra brace at the end of the file
while content.strip().endswith("}"):
    content = content[:content.rfind("}")]

# Put one valid brace back for loop() if we accidentally stripped it, let's just do it cleanly:
lines = content.split('\n')
for i in range(len(lines)-1, -1, -1):
    if lines[i].strip() == "}":
        pass

# Instead of parsing, let's just write the whole .ino file clean, it's safer.
