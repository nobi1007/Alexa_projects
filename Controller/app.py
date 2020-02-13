from flask import Flask
from flask_ask import session, question, statement, Ask
from controllerTemplates import query

app = Flask(__name__)

@app.route("/")
def homePage():
    return "This is the home page where all the utterances will be shown."

ask = Ask(app,"/")

@ask.launch
def intro():
    response = query["intro"]
    return question(response)

@ask.intent("aboutIntent",convert={'aboutVals':str})
def description(aboutVals):
    if aboutVals == "it" or aboutVals == "this device" or aboutVals == "auto wheels": 
       response = query["description"]
    else:
        response = query["invalidDescription"]
    return question(response)

@ask.intent("movementIntent",convert={'direction':str})
def frontBackMovement(direction):
    if direction == "forward" or direction == "backward":
        response = query["movementLinear"].format(direction)
    elif direction == "left" or direction == "right":
        response = query["movementSide"].format(direction)
    else:
        response = query["invalidMovement"]
    return(question(response))

# @ask.intent("sideMovement",convert={'directionS':str})
# def leftRightMovement(directionS):
#     if directionS == "left":
#         response = query["sideMovementLeft"]
#     elif directionS == "right":
#         response = query["sideMovementRight"]
#     else:
#         response = query["invalidMovement"]
#     return(question(response))

@ask.intent("AMAZON.HelpIntent")
def help():
    response = query["help"]
    return(question(response))

@ask.intent("AMAZON.StopIntent")
def stop():
    response = query["stop"]
    return(statement(response))

@ask.intent("AMAZON.FallbackIntent")
def fallback():
    response = query["fallback"]
    return(question(response))

if __name__ == "__main__":
    app.run(port=4000,debug=True)
