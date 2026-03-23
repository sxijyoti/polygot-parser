def sum(a, b):
    return a + b

class Calculator:
    def multiply(self, a, b):
        return a * b

DATA = {"foo": 1}

def sub(a, b):
    return a - b

def mul(a, b):
    return a * b

def div(a, b):
    if b == 0:
        raise ValueError("Cannot divide by zero")
    return a / b
