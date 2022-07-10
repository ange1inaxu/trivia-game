[demonstration video](https://youtu.be/vZ-0tBB5jtQ)

# Overview

## GET from open-trivia API
I decided to choose solely T/F from the easy category in general knowledge. In the python script, I first checked if the request method was 'GET'. In the case that it was, I checked if scoreboard was passed as an argument. If it wasn't, I defaulted it to False instead of throwing an error and stopping the game as a design choice. Next I used the python requests library to fetch the data from the open Trivia API ("https://opentdb.com/api.php?amount=1&category=9&difficulty=easy&type=boolean") using the pre-selected categories I mentioned. From here, I parsed for 'question' and 'correct_answer' and returned them as a well-formatted string '{question}\n{answer}' to the server to enable easier parsing in our C script later.

## FSM for User Play
Using the Button class from a previous regular exercise (which returns 1 for a short press and 2 for a long press), I initialized button1 (controls the state transitions), button2 (False), and button3 (True). Our state first starts in IDLE, telling the user "Long press (button 1) to start the game!" Once the button1 value is read as 2 (long press), we transitioned to QUESTION_DISPLAY state. Using strtok, I used the '\n' delimiter (as we previously formatted) to parse for question and answer, storing them in their respective variables, and printing the question to the TFT. From here, we transitioned automatically to USER_ANSWER. button2 was designated False and button3 was designated True. We checked if the user's button press matched the correct answer. I updated the local correct and incorrect variables accordingly to keep track of their counts. From here, we transitioned to the UPDATE state. In the case that button1 is short-pressed, we return to state QUESTION_DISPLAY to allow the user to continuously play. If button1 is long-pressed, we transition to the END_GAME state, where we store the current score (correct - incorrect), timestamp, and user to the database on our local server, which we will detail further in the Database Storage section. The local variables, correct and incorrect, are reset. The scoreboard is displayed in descending order, with respect to score. And the state machine returns to the IDLE state and waits for the user to long press button1 again to restart the game.

## Database Storage
Within the python script, we check if request method is 'POST'. If it is, we check if user and score are passed in as valid arguments. If it isn't we throw an exception "Please input a valid user and score argument". Otherwise we create a scoreboard_db with columns timestamp, user, score (if not already created). When the POST request is called on in our FSM in END_GAME state, we insert the current timestamp, user name, and current game score into the table. In the POST request, we automatically select all the columns from the table and sort the score in descending order and print these values to the TFT screen. This allows for persistent storage.