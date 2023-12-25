import pandas as pd
import numpy as np
import util
from itertools import cycle
import random
import string
import uuid


def gen_price(securities):
    
    num_prices_per_security = 1000
    min_price = 99
    max_price = 101
    price_increment = 1 / 256  # Smallest increment
    min_spread = 1 / 128
    max_spread = 1 / 64
    spread_increment = 1 / 256  # Increment for bid/offer spread

    # Function to generate prices and spreads
    def generate_prices_and_spreads():
        # Oscillating prices between min_price and max_price
        prices = np.arange(min_price, max_price, price_increment)
        prices = np.concatenate([prices, np.arange(max_price, min_price, -price_increment)])  # Append reverse to oscillate
        full_prices = np.tile(prices, num_prices_per_security // len(prices) + 1)[:num_prices_per_security]

        # Oscillating spreads between min_spread and max_spread
        spreads = np.arange(min_spread, max_spread, spread_increment)
        spreads = np.concatenate([spreads, np.arange( max_spread,min_spread, -spread_increment)])  # Append reverse to oscillate
        full_spreads = np.tile(spreads, num_prices_per_security // len(spreads) + 1)[:num_prices_per_security]

        return full_prices, full_spreads

    # Generate data for each security
    all_data = pd.DataFrame()

    for security in securities:
        prices, spreads = generate_prices_and_spreads()
        data = pd.DataFrame({'Instrument': security, 'Price': prices, 'Bid/Offer Spread': spreads})
        all_data = pd.concat([all_data, data])

    all_data['Price'] = all_data.Price.apply(util.decimal_to_fractional)

    # Reset index
    all_data.reset_index(drop=True, inplace=True)

    txt_filename = 'prices.txt'
    
    all_data.to_csv(txt_filename, header= False, index=False)
    
    all_data.to_json(orient='records')

    return all_data

def generate_random_string(length=10):
    """ Generate a random string of fixed length """
    letters = string.ascii_letters
    return ''.join(random.choice(letters) for i in range(length))

def gen_trade_data(securities):
    
    num_trades_per_security = 10
    
    books = ["TRSY1", "TRSY2", "TRSY3"]
    quantities = [1000000, 2000000, 3000000, 4000000, 5000000]
    trade_actions = ["BUY", "SELL"]
    generated_trade_ids = set()

    trades = []
    for security in securities:
        quantity_cycle = cycle(quantities)
        trade_action_cycle = cycle(trade_actions)

        for _ in range(num_trades_per_security):
            action = next(trade_action_cycle)

            # Ensure unique TradeID
            trade_id = generate_random_string()
            while trade_id in generated_trade_ids:
                trade_id = generate_random_string()

            generated_trade_ids.add(trade_id)

            trade = {
                "Security": security,
                "TradeID": trade_id,
                "Price": 99.0 if action == "BUY" else 100.0,
                "Book": random.choice(books),
                "Quantity": next(quantity_cycle),
                "Action": action
            }
            trades.append(trade)
    
    trade_data = pd.DataFrame(trades)    
    trade_data['Price'] = trade_data.Price.apply(util.decimal_to_fractional)

    txt_filename = 'trades.txt'
    
    trade_data.to_csv(txt_filename, header= False, index=False)
    
    return trade_data

def gen_market_data_final(securities):
    # Constants
    num_updates = 1000
    size_multiplier = 10000000  # Size at each level (multiplier of 10 million)
    levels = 5  # Number of levels deep for bid and offer

    # Spread starts at 1/128 and widens by the smallest increment on each update
    spread_start = 1/128
    spread_increment = 1/128
    spread_widening = 1/128
    spread_max = 1/32
    spread_reset = 1/128

    # Price oscillation setup
    mid_price_start = 99
    mid_price_end = 101
    price_increment = 1/256  # US Treasuries trade in 1/256th increments

    # Generate the oscillating mid prices
    def generate_mid_prices(start, end, increment, num_updates):
        prices = []
        price = start
        direction = 1  # Start with incrementing the price

        for _ in range(num_updates):
            prices.append(price)
            next_price = price + direction * increment

            if next_price > end or next_price < start:
                direction *= -1  # Change direction
                next_price = price + direction * increment

            price = next_price

        return prices

    # Generate spreads for top of book that widen and narrow
    def generate_spreads(spread_start, spread_increment, spread_widening, spread_max, spread_reset, num_updates):
        spreads = []
        spread = spread_start

        for _ in range(num_updates):
            spreads.append(spread)
            if spread < spread_max and not spreads[-1] == spread_reset:
                spread += spread_widening
            else:
                spread = spread_reset

        return spreads

    # Initialize the DataFrame
    df_data = []

    # Generate order book updates
    for security in securities:
        mid_prices = generate_mid_prices(mid_price_start, mid_price_end, price_increment, num_updates)
        spreads = generate_spreads(spread_start, spread_increment, spread_widening, spread_max, spread_reset, num_updates)

        for update in range(num_updates):
            for level in range(1, levels + 1):
                level_spread = spreads[update] + (level - 1) * spread_increment
                bid_size = offer_size = level * size_multiplier

                df_data.append({
                    'Security': security,
                    'Update': update + 1,
                    'Price': mid_prices[update],
                    'Spread': level_spread,
                    'Level': level,
                    'Bid Size': bid_size,
                    'Offer Size': offer_size
                })
        print(security)
    # Create DataFrame
    order_book_df = pd.DataFrame(df_data)
    
    order_book_df['Tenor'] = order_book_df.Security.str.removesuffix('Y').apply(int)
    order_book_df.sort_values(['Update', 'Tenor','Level'], inplace = True)

    order_book_df['Price'] = order_book_df.Price.apply(util.decimal_to_fractional)

    txt_filename = 'marketdata.txt'
    
    order_book_df.drop(columns=['Update', 'Tenor', 'Level']).to_csv(txt_filename, header= False, index=False)

    return order_book_df


# Function to create inquiries with US Treasuries price convention
def gen_inquiries(securities):
    inquiries = []
    for security in securities:
        for _ in range(10):  # 10 inquiries per security
            inquiry_id = str(uuid.uuid4())  # Generate a unique long string ID
            side = random.choice(["BUY", "SELL"])
            quantity = random.choice(range(100000, 201000, 1000))

            # Generate price in terms of points and fractions
            points = random.randint(99, 101)
            fraction = random.randint(0, 255)  # Fractional part as 1/256 of a point
            price = points + fraction / 256.0  # Convert to decimal system

            inquiries.append({
                "inquiryId": inquiry_id,
                "security": security,
                "side": side,
                "quantity": quantity,
                "price": round(price, 8)  # Round to 8 decimal places for clarity
            })
    df = pd.DataFrame(inquiries)
    df['price'] = df.price.apply(util.decimal_to_fractional)

    txt_filename = 'inquiries.txt'
    
    df.to_csv(txt_filename, header= False, index=False)
    return df


if __name__ == '__main__':

    securities = ["2Y", "3Y", "5Y", "7Y", "10Y", "20Y", "30Y"]

    data = gen_price(securities)

    trade_data_df = gen_trade_data(securities)

    trade_data_df

    # Generate and display the final corrected dataset
    final_order_book_df = gen_market_data_final(securities)
    final_order_book_df.head(10)

    gen_inquiries(securities)
    
    print('done')