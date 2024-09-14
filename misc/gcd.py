from gcd2 import calc_gcd

def main():

    num1 = 56
    num2 = 98
    narr = [1, 2, 3, 4, 5]
    str1 = 'Hello, world!'

    num1 += num2;
    result_gcd = calc_gcd(num1, num2)
    print(f"The GCD of {num1} and {num2} is {result_gcd}.")
    print(f"Hello, world!\nThis is the pdb debugger test.")

    total = 0
    for n in narr:
        total += n
    print(f"The total is {total}.")
    

if __name__ == "__main__":
    main()

