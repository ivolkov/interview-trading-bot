from datetime import datetime
from dateutil import parser
from pychart import theme, axis, area, line_style, line_plot, tick_mark


def parse_files(balance_data, prices_data, profit_data):
    balance_log = open('balance.log', 'r')
    prices_log = open('prices.log', 'r')

            # parse balance.log file
    for line in balance_log:
        words = line.split()
        
        if len(words) < 5:
            continue

                # search for datetime stamp
        datetime_val = parser.parse(words[0] + ' ' + words[1] + ' ' + words[2])
        
        days_diff = - (datetime.now() - datetime_val).total_seconds() / 86400.0

                # search for bot number
        bot_num_str = words[4][6:8]
        if bot_num_str[1] == '[':
            bot_num_str = bot_num_str[0]
        bot_num = int(bot_num_str)

                # search for balance value
        balance_val = float(words[5])         

                # add new word to dictionary balance_data
        if bot_num not in balance_data:
            balance_data[bot_num] = list()

                # append balance data
        balance_data[bot_num].append((days_diff, balance_val))
        
                #search for calculated profit value
        calc_profit = 0.0
        if len(words) > 6:
            calc_profit = float(words[6])    
        
                # add new word to dictionary calc_profit
        if bot_num not in profit_data:
            profit_data[bot_num] = 0.0
            
                # append calculated profit 
        profit_data[bot_num] += calc_profit
        
            # temp code: statictics
    total_calc_profit = 0.0
    for data in profit_data:
        total_calc_profit += data
            
    total_balance_diff = 0.0
    for data in balance_data:
        bot_num = data
        first_balance = balance_data[bot_num][0][1]
        last_balance = balance_data[bot_num][-1][1]
        balance_diff = last_balance - first_balance
        print('Bot #' + str(bot_num) + ': ' + str(len(balance_data[bot_num])) + ' trades, ' + str(balance_diff) + ' diff, ' + str(profit_data[bot_num]) + ' calculated profit')
        total_balance_diff += balance_diff
        
    print('Total balance diff ' + str(total_balance_diff))
       
            # parse prices.log file 
    for line in prices_log:
        words = line.split()
        
        if len(words) < 7:
            continue
        
                # search for datetime stamp
        datetime_val = parser.parse(words[0] + ' ' + words[1] + ' ' + words[2])
        days_diff = - (datetime.now() - datetime_val).total_seconds() / 86400.0
        
                # search for currency value
        currency_str = words[6]
        
                # add currency to dictionary prices_data
        if currency_str not in prices_data:
            prices_data[currency_str] = list()
        
                # search for price value
        price_val = float(words[7])
        
        prices_data[currency_str].append((days_diff, price_val))
    
            # close files    
    balance_log.close()
    prices_log.close()
        
    return balance_data, prices_data


def make_plot(balance_data, price_data):
    plot_width = 800
    plot_height = 500
    x_interval = 2
    y1_interval = 1
    y2_interval = 10
         
        # prepare pychart
    theme.output_format = 'pdf'
    theme.output_file = 'trades.pdf'
    theme.use_color = True
    #theme.scale_factor = 2.0
    #theme.default_font_size = 6
    #theme.default_line_height = 25
    #theme.default_line_width = 1
    theme.reinitialize()    
    
        #btc
    xaxis_btc = axis.X(label='', \
                       format='%.1f', \
                       tic_interval=x_interval)
    
    yaxis_balance_btc = axis.Y(label='', \
                               format='%.2f', \
                               tic_interval=y1_interval)
    
    yaxis_price_btc = axis.Y(label='', \
                             format='%.2f', \
                             offset=plot_width, \
                             label_offset=(65,0), \
                             tic_len=-6, \
                             tic_interval=y2_interval)
    
    area_balance_btc = area.T(size=(plot_width, plot_height), \
                              x_axis=xaxis_btc, \
                              y_axis=yaxis_balance_btc, \
                              x_range=(None, 0), \
                              y_grid_style=None, \
                              x_grid_interval=x_interval, \
                              y_grid_interval=y1_interval)
    
    area_price_btc = area.T(size=(plot_width, plot_height), \
                            x_axis=None, \
                            y_axis=yaxis_price_btc, \
                            x_range=(None, 0), \
                            legend=None, \
                            x_grid_style=line_style.gray70_dash3, \
                            y_grid_interval=y2_interval)
    
    pair = "btc_usd"   
    if pair in price_data:
        area_price_btc.add_plot(line_plot.T(label='', data=price_data[pair]))
    
    for bot_num in range(12):
        if bot_num in balance_data:
            area_balance_btc.add_plot(line_plot.T(label='trader ' + str(bot_num), data=balance_data[bot_num], tick_mark=tick_mark.circle2))
        
    area_balance_btc.draw()
    area_price_btc.draw()
         
    return


if __name__ == '__main__':        
    balance_data = dict()
    price_data = dict()
    profit_data = dict()
       
    parse_files(balance_data, price_data, profit_data)
    make_plot(balance_data, price_data)
    make_plot(balance_data, price_data)      
    