import os
import json
import random
from flask import Blueprint, redirect, render_template, request, url_for, flash
import requests

views = Blueprint('views', __name__)

global_username = ""

# Site Routing

# Homepage
@views.route('/')
def index():
    return redirect(url_for('views.login'))

# Dashboard
@views.route('/dashboard', methods=['GET', 'POST'])
def dashboard():
    if request.method == "POST":
        try:
            if "remote_unlock" in request.form:
                # send the request to thingspeak
                apiWriteKey = "Q3X0NH6PSG5W6DO1"
                getStr = "/update?api_key="
                getStr += apiWriteKey
                getStr += "&field4=1"
                payload = "GET https://"
                payload += "api.thingspeak.com" + getStr
                payload += "\r\n"
                x = requests.get(payload)
                print(x.status_code)

            else:
                pass
        except:  
            pass # if any error,such as crashes, we just pass so that whole programme will continue.
        
    return render_template('dashboard.html')

# Login Page
@views.route('/login', methods=["GET", "POST"])
def login():
    error = ""
    if request.method == "POST":
        username = request.form['username']
        password = request.form['password']
        
        if username == "admin":
            if password == "password":
                global global_username
                global_username = username
                return redirect(url_for('views.dashboard'))
            else:
                error = "Password is incorrect"
        else:
            error = "Username does not exist"

    return render_template('login.html', error=error)

@views.route('/profile')
def profile():
    username = global_username

    return render_template('profile.html', username=username)

