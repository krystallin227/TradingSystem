def decimal_to_fractional(decimal):
    whole_part = int(decimal)
    fractional_part = decimal - whole_part

    # Convert the fractional part to 32nds
    fraction_32 = fractional_part * 32
    fraction_32_whole = int(fraction_32)
    fraction_32_remainder = fraction_32 - fraction_32_whole

    # Convert the remainder to 256ths
    fraction_256 = int(round(fraction_32_remainder * 8))  # 8 is 256/32

    # Handle special case for 4 (represented as '+')
    fraction_256_str = '+' if fraction_256 == 4 else str(fraction_256) if fraction_256 != 0 else ''

    return f"{whole_part}-{fraction_32_whole:02}{fraction_256_str}"


def fractional_to_decimal(fractional):
    whole_part, fraction_part = fractional.split('-')

    # Extract parts for 32nds and 256ths
    fraction_32 = int(fraction_part[:2])
    fraction_256 = 4 if len(fraction_part) > 2 and fraction_part[2] == '+' else int(fraction_part[2:]) if len(fraction_part) > 2 else 0

    # Convert to decimal
    decimal_fraction = fraction_32 / 32 + fraction_256 /  256

    return float(whole_part) + decimal_fraction

if __name__ == '__main__':
    # Example Usage
    print(decimal_to_fractional(100.00390625))  # Should output 100-001
    print(decimal_to_fractional(100.796875))    # Should output 100-25+
    print(fractional_to_decimal("100-001"))     # Should output 100.00390625
    print(fractional_to_decimal("100-25+"))     # Should output 100.796875
