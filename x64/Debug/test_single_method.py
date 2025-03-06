import logging

def test_function_1(x: int, y: int):
    logging.info(f"Adding {x} and {y}.")
    z = x + y
    logging.info(f"The result is {x}.")
    return z ** 2