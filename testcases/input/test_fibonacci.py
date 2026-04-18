a = 0
b = 1
count = 0
max = 10

print("First 10 numbers:")
while count < max:
    print(a)
    temp = a + b
    a = b
    b = temp
    count = count + 1

print("Done!")
