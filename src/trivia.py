#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Thu Mar  3 12:40:45 2022

@author: angelinaxu
"""
import requests
import json
import pprint #prettyprint in python
import html
import sqlite3
import datetime


# Example: {'method': 'GET', 'args': ['lat', 'lon'], 'values': {'lat': '11', 'lon': '5'}}
def request_handler(request):
    
    output = ""
    
    if request['method'] == 'GET':
        
        scoreboard = request["values"].get("scoreboard", "False")
        
        if scoreboard == "False":
            r = requests.get("https://opentdb.com/api.php?amount=1&category=9&difficulty=easy&type=boolean")
            response = json.loads(r.text)
            pprint.pprint(response)
            question = html.unescape(response['results'][0]['question'])
            answer = html.unescape(response['results'][0]['correct_answer'])
            output = f"{question}\n{answer}"
        
        elif scoreboard == "True":
            scoreboard_db = "/var/jail/home/angelx15/scoreboard.db" # just come up with name of database
            conn = sqlite3.connect(scoreboard_db)  # connect to that database (will create if it doesn't already exist)
            c = conn.cursor()  # move cursor into database (allows us to execute commands)
            things = c.execute('''SELECT * FROM scoreboard ORDER BY score DESC;''').fetchall()
            output += "\n\n"
            for x in things:
                output += f"Timestamp: {x[0][:-10]} || User: {x[1]} || Score: {x[2]}\n"
            conn.commit() # commit commands
            conn.close() # close connection to database
        
    elif request['method'] == 'POST':
        try:
            user = request['form']['user']
            score = int(request['form']['score'])
        except:
            return "Please input a valid user and score argument"
        
        scoreboard_db = "/var/jail/home/angelx15/scoreboard.db" # just come up with name of database
        conn = sqlite3.connect(scoreboard_db)  # connect to that database (will create if it doesn't already exist)
        c = conn.cursor()  # move cursor into database (allows us to execute commands)
        
        c.execute('''CREATE TABLE IF NOT EXISTS scoreboard (time_stamp timestamp, user text, score int);''') # run a CREATE TABLE command
        c.execute('''INSERT into scoreboard VALUES (?,?,?);''', (datetime.datetime.now(), user, score))
                    
        things = c.execute('''SELECT * FROM scoreboard ORDER BY score DESC;''').fetchall()
        output += "\n\n"
        for x in things:
            output += f"Timestamp: {x[0][:-10]} || User: {x[1]} || Score: {x[2]}\n\n"
        conn.commit() # commit commands
        conn.close() # close connection to database
            
    return output