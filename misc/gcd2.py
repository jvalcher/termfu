
def calc_gcd(a, b):

    while b != 0:
        a, b = b, a % b
    return a

