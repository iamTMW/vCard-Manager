# Name: Taha Mohyuddin
# Student ID: 1275575 
# Date: 27 - Mar - 2025
# Course: CIS2750


#!/usr/bin/env python3

import os
import sys
import ctypes 
import mysql.connector
import re
from mysql.connector import connect, Error
from asciimatics.widgets import Frame, ListBox, Layout, Divider, Text, Button, Widget, PopUpDialog  # Needed for error popups
from asciimatics.scene import Scene
from asciimatics.screen import Screen
from asciimatics.exceptions import ResizeScreenError, NextScene, StopApplication  
from datetime import datetime

# This is another helper 
def raise_(ex):
    raise ex


# The Below Class VCardMan, will manages the loading, validating, and interacting with vCard files using a C shared library.
class VCardMan: 
    #The below def initializes the VCardMan with paths to the C shared library and card directory. Loads the shared library and sets up function signatures.
    def __init__(self, lib_path="./libvcparser.so", card_dir="./cards"):
        self.card_dir = card_dir
        self.cards = []  # List of (label, filename)
        self.current_file = None

        # Below it will load the C library
        self.lib = ctypes.CDLL(lib_path)

        self.lib.valiF.argtypes = [ctypes.c_char_p]
        self.lib.valiF.restype = ctypes.c_int

        self.lib.getFNFromFile.argtypes = [ctypes.c_char_p]
        self.lib.getFNFromFile.restype = ctypes.c_char_p

        self.lib.CardSumm.argtypes = [ctypes.c_char_p]
        self.lib.CardSumm.restype = ctypes.c_char_p

        self.lib.upFN.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
        self.lib.upFN.restype = ctypes.c_int 
        self.lib.BDAY_File.argtypes = [ctypes.c_char_p]
        self.lib.BDAY_File.restype = ctypes.c_char_p

        self.lib.ANNIV_File.argtypes = [ctypes.c_char_p]
        self.lib.ANNIV_File.restype = ctypes.c_char_p 
        self.lib.getOptP.argtypes = [ctypes.c_char_p]
        self.lib.getOptP.restype = ctypes.c_char_p 
        self.lib.createCard.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
        self.lib.createCard.restype = ctypes.c_int 
        self.lib.createNewC.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
        self.lib.createNewC.restype = ctypes.c_int

        self.load_cards()  # This will load all valid cards into memory

    # The Below def will loads all valid .vcf or .vcard files from the directory, validates each file, and stores summary info into self.cards.
    def load_cards(self):
        self.cards.clear()
        for file in os.listdir(self.card_dir):
            if file.endswith(".vcf") or file.endswith(".vcard"):
                path = os.path.join(self.card_dir, file).encode("utf-8")
                if self.lib.valiF(path) == 0:
                    summ = self.lib.CardSumm(path) # Here i am declaring summ stands for summary
                    if summ:
                        label = summ.decode("utf-8")
                        self.cards.append((file , file))

    # The Below def will return a list of label, filename pairs for all loaded vCard files.
    def get_summary(self):
        return [(label, filename) for label, filename in self.cards]

    #The Below def will returns the full name (FN) from the specified vCard file.
    def get_fn(self, filename):
        path = os.path.join(self.card_dir, filename).encode("utf-8")
        result = self.lib.getFNFromFile(path)
        return result.decode("utf-8") if result else "N/A"

    #The Below def udates the full name (FN) in the specified vCard file. Returns 0 if success, or error code otherwise.
    def update_fn(self, filename, new_fn):
        path = os.path.join(self.card_dir, filename).encode("utf-8")
        return self.lib.upFN(path, new_fn.encode("utf-8"))

# The below class LogV, will handle the MySQL login screen, allowing the user to connect to the database.
class LogV(Frame): 
    # The below def will create the login form UI with Username, Password, and Database input fields.
    def __init__(self, screen):
        super(LogV, self).__init__(screen, screen.height * 2 // 3, screen.width * 2 // 3, hover_focus=True, title="MySQL Login")
        self.db_conn = None
        self.error_message = "" 
        # The below will create the login form UI with Username, Password, and Database input fields.
        layout = Layout([100], fill_frame=True)
        self.add_layout(layout)
        
        # The below will input fields for user credentials
        layout.add_widget(Text("Username:", "user", on_change=self.clear_error))
        layout.add_widget(Text("Password:", "password", hide_char="*", on_change=self.clear_error))
        layout.add_widget(Text("Database:", "database", on_change=self.clear_error))
        layout.add_widget(Divider())

        # The below is the layout for buttons
        layout2 = Layout([1, 1])
        self.add_layout(layout2)
        layout2.add_widget(Button("Connect", self._connect), 0)
        layout2.add_widget(Button("Quit", self._quit), 1)

        self.fix() # This will finalize layout

    # The following below will clear any existing error message when the user starts typing.
    def clear_error(self):
        self.error_message = ""

    # The following below will try to connect to the MySQL database using the entered credentials. That said, it will show an error popup if connection fails.
    def _connect(self):
        self.save()
        creds = self.data

        try:
            self.db_conn = connect(
                host="dursley.socs.uoguelph.ca",
                user=creds["user"],
                password=creds["password"],
                database=creds["database"]
            )
            self.screen._db_conn = self.db_conn   # This will make the DB available globally so connection globally
            raise NextScene("Main") # This will, go to the main screen
        except Error as e:
            self.error_message = f"Oops, Connection failed: {e}"
            self.scene.add_effect(
                PopUpDialog(self.screen, self.error_message, ["OK"])
            )

    @staticmethod 
    # The below def will quit the app if the user presses the Quit button.
    def _quit():
        raise StopApplication("User exited login screen")

# The below will format the date
def format_date(date_str):
    if date_str and len(date_str) == 8:
        return f"{date_str[:4]}-{date_str[4:6]}-{date_str[6:]} 00:00:00"
    return None 
# The below will parse the date time
def parse_date_time(raw):
        if not raw:
            return ""
        decoded = raw.decode()
        match = re.search(r"Date:\s*(\d+),\s*Time:\s*(\d+),\s*UTC:\s*(\d),", decoded)
        if match:
            date = match.group(1)
            time = match.group(2)
            utc_flag = match.group(3)
            return f"Date: {date} Time: {time}" + (" (UTC)" if utc_flag == "1" else "")
        return decoded  # fallback
# The below class ListView, is the main menu view that displays all valid vCard files and allows user interaction like edit, create, query DB, quit
class ListView(Frame): 
    # The below def will set up the main list view for browsing and interacting with vCard files.
    def __init__(self, screen, model):
        super(ListView, self).__init__(screen, screen.height * 2 // 3, screen.width * 2 // 3, on_load=self._reload_list, hover_focus=True, can_scroll=False, title="vCard File List")
        self._model = model

        # Below is the vCard file list UI
        self._list_view = ListBox(
            Widget.FILL_FRAME,
            model.get_summary(),
            name="vcards",
            add_scroll_bar=True,
            on_change=self._on_pick,
            on_select=self._edit)
        self._edit_button = Button("Edit", self._edit)

        # Below is the Layout for list and buttons
        layout = Layout([100], fill_frame=True)
        self.add_layout(layout)
        layout.add_widget(self._list_view)
        layout.add_widget(Divider())

        # Below is the one layout with four equal width columns
        button_layout = Layout([1, 1, 1, 1])
        self.add_layout(button_layout)
        button_layout.add_widget(Button("Create", self.myCre), 0)
        button_layout.add_widget(self._edit_button, 1)
        button_layout.add_widget(Button("DB Queries", self.quedb), 2)
        button_layout.add_widget(Button("Quit", self._quit), 3)


        self.fix()
        self._on_pick()
    
    # The following below def will go to the database query screen
    def quedb(self):
        raise NextScene("DB Queries")

    # The following below def will go to the Create New vCard screen.
    def myCre(self):
        self._model.current_file = None  # This will indicate creation mode
        raise NextScene("Create")

    # Enable the Edit button only if a vCard file is selected.
    def _on_pick(self):
        self._edit_button.disabled = self._list_view.value is None

    # The following def extRD is an helper method to extract raw date YYYYMMDD from a decoded date string.
    def extRD(raw_str):
        if raw_str:
            match = re.search(r'Date:\s*(\d{8})', raw_str.decode())
            if match:
                return match.group(1)
        return None

    # The following def below will reload and refresh the list of valid vCard files. Which also performs automatic database sync and repopulation.
    def _reload_list(self, new_value=None): 
        def extRD(raw_str):
            if raw_str:
                match = re.search(r'Date:\s*(\d{8})', raw_str.decode())
                if match:
                    return match.group(1)
            return None

        self._model.load_cards()
        self._list_view.options = self._model.get_summary()
        self._list_view.value = new_value 
        # it will Automatically sync DB
        try: 
            conn = self.screen._db_conn
            cursor = conn.cursor() 
            
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS FILE (
                    file_id INT AUTO_INCREMENT PRIMARY KEY,
                    filename VARCHAR(60) NOT NULL,
                    last_modified DATETIME,
                    creation_time DATETIME NOT NULL
                )
            ''')

            cursor.execute('''
                CREATE TABLE IF NOT EXISTS CONTACT (
                    contact_id INT AUTO_INCREMENT PRIMARY KEY,
                    name VARCHAR(256) NOT NULL,
                    birthday DATETIME,
                    anniversary DATETIME,
                    file_id INT NOT NULL,
                    FOREIGN KEY (file_id) REFERENCES FILE(file_id) ON DELETE CASCADE
                )
            ''')


            # Below t will Ccear these tables too to keep them in sync
            cursor.execute("DELETE FROM CONTACT")
            cursor.execute("DELETE FROM FILE")


            # Below it will create tables if they don't exist
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS vcard_data (
                    id INT AUTO_INCREMENT PRIMARY KEY,
                    filename VARCHAR(255),
                    fn TEXT,
                    bday TEXT,
                    anniversary TEXT,
                    prop_count INT
                )
            ''') 
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS optional_props (
                    id INT AUTO_INCREMENT PRIMARY KEY,
                    vcard_filename VARCHAR(255),
                    prop_name VARCHAR(255),
                    prop_value TEXT
                )
            ''')

            # The Below will clear previous data
            cursor.execute("DELETE FROM vcard_data")
            cursor.execute("DELETE FROM optional_props")

            # Below it will Re insert all cards
            for label, filename in self._model.cards:
                path = os.path.join(self._model.card_dir, filename).encode("utf-8")
                fn = self._model.get_fn(filename)
                bday_raw = self._model.lib.BDAY_File(path)
                anniv_raw = self._model.lib.ANNIV_File(path)

                bday = extRD(bday_raw)
                anniv = extRD(anniv_raw)

                match = re.search(r'Props:\s*(\d+)', label)
                count = int(match.group(1)) if match else 0

                cursor.execute('''
                    INSERT INTO vcard_data (filename, fn, bday, anniversary, prop_count)
                    VALUES (%s, %s, %s, %s, %s)
                ''', (filename, fn, bday, anniv, int(count)))

                # Below it will format date fields
                bday_dt = format_date(bday)
                anniv_dt = format_date(anniv)

                # Below it will get file timestamps
                full_path = os.path.join(self._model.card_dir, filename)
                mod_time = os.path.getmtime(full_path)
                mod_str = datetime.fromtimestamp(mod_time).strftime('%Y-%m-%d %H:%M:%S')

                # Below it will insert into FILE table
                cursor.execute('''
                    INSERT INTO FILE (filename, last_modified, creation_time)
                    VALUES (%s, %s, %s)
                ''', (filename, mod_str, mod_str))

                # Beow it will get the file_id foreign key
                file_id = cursor.lastrowid

                # Below it will insert into CONTACT table
                cursor.execute('''
                    INSERT INTO CONTACT (name, birthday, anniversary, file_id)
                    VALUES (%s, %s, %s, %s)
                ''', (fn, bday_dt, anniv_dt, file_id))



                optional = self._model.lib.getOptP(path)
                if optional:
                    for line in optional.decode().splitlines():
                        if ':' in line:
                            key, value = line.split(':', 1)
                            cursor.execute('''
                                INSERT INTO optional_props (vcard_filename, prop_name, prop_value)
                                VALUES (%s, %s, %s)
                            ''', (filename, key.strip(), value.strip()))

            conn.commit()

        except Exception as e: 
            # The below will show error popup if DB sync fails
            self.scene.add_effect(PopUpDialog(self.screen, f"Auto DB Sync Failed: {e}", ["OK"]))

    # The below def will navigate to the contact detail screen for the selected vCard.
    def _edit(self):
        self.save()
        self._model.current_file = self.data["vcards"]
        raise NextScene("Details") 
    

    # The below def will exit the application when user chooses to quit.
    @staticmethod
    def _quit():
        raise StopApplication("User pressed quit")


# The below class ContactView, will let the user edit the full name (FN) of the selected vCard.
class ContactView(Frame): 
    # The below def will initialize the ContactView screen with a text field for editing FN and Save/Cancel buttons.
    def __init__(self, screen, model):
        super(ContactView, self).__init__(screen, screen.height * 2 // 3, screen.width * 2 // 3, hover_focus=True, can_scroll=False, title="Edit Full Name")
        self._model = model # This model will hold the current vCard info

        # Below is the layout for FN text input
        layout = Layout([100], fill_frame=True)
        self.add_layout(layout)
        layout.add_widget(Text("Full Name (FN):", "fn"))
        
        # Below is the layout for Save and Cancel buttons
        layout2 = Layout([1, 1])
        self.add_layout(layout2)
        layout2.add_widget(Button("Save", self._save), 0)
        layout2.add_widget(Button("Cancel", self._cancel), 1)
        self.fix()

    # The below def reset will fill the FN text box with current file’s FN.
    def reset(self):
        super(ContactView, self).reset()
        current_fn = self._model.get_fn(self._model.current_file)
        self.data = {"fn": current_fn}

    # The below def will save the updated FN back to the vCard file and return to main screen.
    def _save(self):
        self.save()
        new_fn = self.data["fn"]
        self._model.update_fn(self._model.current_file, new_fn)
        raise NextScene("Main")
    # The below def will cancel editing and return to the main screen without saving.
    @staticmethod
    def _cancel():
        raise NextScene("Main") 

# The below class DBQueV, will handle displaying the results from database queries.
class DBQueV(Frame): 
    # The below def will initializes the DB Query view with result box and buttons for queries.
    def __init__(self, screen):
        super(DBQueV, self).__init__(screen, screen.height * 2 // 3, screen.width * 2 // 3, hover_focus=True, title="DB Queries")

        # The below is the ListBox to display results from the DB
        self._results_box = ListBox(
            Widget.FILL_FRAME,
            options=[("Results will appear here", 0)],
            name="query_results",
            add_scroll_bar=True
        )

        # Below is the main layout for results
        layout = Layout([100], fill_frame=True)
        self.add_layout(layout)
        layout.add_widget(self._results_box)
        layout.add_widget(Divider())

        # Below is the Layout for buttons at bottom
        layout2 = Layout([1, 1, 1])
        self.add_layout(layout2)
        layout2.add_widget(Button("Display all contacts", self.showE), 0) 
        layout2.add_widget(Button("Find contacts born in June", self.juneBirth), 1)
        layout2.add_widget(Button("Cancel", self._cancel), 2)

        self.fix()

    # The Below def queries all contacts from vcard_data and displays their filename and FN.
    def showE(self):
        try:
            conn = self.screen._db_conn
            cursor = conn.cursor()
            cursor.execute('''
                SELECT CONTACT.contact_id, CONTACT.name, CONTACT.birthday,
                CONTACT.anniversary, FILE.filename
                FROM CONTACT
                JOIN FILE ON CONTACT.file_id = FILE.file_id
                ORDER BY CONTACT.name
            ''')

            rows = cursor.fetchall()

            if not rows:
                self._results_box.options = [("No contacts found", 0)]
            else:
                # Header row with aligned column titles
                header = f"{'ID':<4} | {'Name':<20} | {'BDAY':<20} | {'ANNIV':<20} | {'FILE':<25}"
                separator = "-" * len(header)

                # Build the formatted contact list
                self._results_box.options = [(header, 0), (separator, 1)] + [
                    (f"{cid:<4} | {name:<20} | {str(bday):<20} | {str(anniv):<20} | {filename:<25}", idx + 2)
                    for idx, (cid, name, bday, anniv, filename) in enumerate(rows)
                ]

        except Exception as e:
            self._results_box.options = [(f"Error: {e}", 0)] 
    # The below def will format dates to MySQL DATETIME
    def format_date(date_str):
        if date_str and len(date_str) == 8:
            return f"{date_str[:4]}-{date_str[4:6]}-{date_str[6:]} 00:00:00"
        return None

        bday_dt = format_date(bday) 
        anniv_dt = format_date(anniv)

        # The below is the file timestamps
        full_path = os.path.join(self._model.card_dir, filename)
        mod_time = os.path.getmtime(full_path)
        mod_str = datetime.fromtimestamp(mod_time).strftime('%Y-%m-%d %H:%M:%S')

        # The below will insert into FILE
        cursor.execute('''
            INSERT INTO FILE (file_name, last_modified, creation_time)
            VALUES (%s, %s, %s)
        ''', (filename, mod_str, mod_str))

        file_id = cursor.lastrowid

        # The below will insert into CONTACT
        cursor.execute('''
            INSERT INTO CONTACT (name, birthday, anniversary, file_id)
            VALUES (%s, %s, %s, %s)
        ''', (fn, bday_dt, anniv_dt, file_id))


    # The below def finds all contacts with birthdays in June and shows filename, FN, and BDAY.
    def juneBirth(self):
        try:
            conn = self.screen._db_conn # This will get active DB connection from screen
            cursor = conn.cursor()  # This will create cursor to execute SQL queries
            cursor.execute('''
                SELECT CONTACT.name, CONTACT.birthday, FILE.filename,
                DATEDIFF(FILE.last_modified, CONTACT.birthday) / 365 AS age
                FROM CONTACT
                JOIN FILE ON CONTACT.file_id = FILE.file_id
                WHERE MONTH(CONTACT.birthday) = 6
                ORDER BY age DESC
            ''')

            rows = cursor.fetchall() # This will fetch all matching rows from the query

            if not rows:  # So if there are no matches, it will display a message
                self._results_box.options = [("No June birthdays found", 0)]
            else:
                # Below is my Header + separator
                header = f"{'Name':<25} | {'BDAY':<12}"
                separator = "-" * len(header)

                self._results_box.options = [(header, 0), (separator, 1)] + [
                    (f"{name:<25} | {bday.strftime('%Y-%m-%d'):<12}", idx + 2)
                    for idx, (name, bday, file_name, age) in enumerate(rows)
                ]

        except Exception as e:
            self._results_box.options = [(f"Error: {e}", 0)]

    # The below def will cancel and go back to main view
    def _cancel(self):
        raise NextScene("Main") 
    # The below def will be called when the screen is loaded again to clear previous state.
    def reset(self):
        super(DBQueV, self).reset()
        self.data = {}  # This will clear any previous data each time scene is loaded
        self._results_box.options = [("No results yet", None)]

# Below is my class CardDetV,it will view to show and edit details for a selected vCard filename, FN, BDAY, ANNIV, props
class CardDetV(Frame):

    # The below def initializes the card detail view, allowing filename and FN to be updated.
    def __init__(self, screen, model):
        super(CardDetV, self).__init__(screen, screen.height * 2 // 3, screen.width * 2 // 3, hover_focus=True, title="vCard Details")
        self._model = model

        layout = Layout([100], fill_frame=True)
        self.add_layout(layout)

        # The below will create fields to show and edit card info
        self._filename = Text("File name:", "filename") # This will be editable 
        self._fn = Text("Contact:", "fn") # This will be editable 
        self._bday = Text("Birthday:", "bday", readonly=True)
        self._anniv = Text("Anniversary:", "anniv", readonly=True)
        self._props = Text("Other properties:", "props", readonly=True)

        layout.add_widget(self._filename)
        layout.add_widget(self._fn)
        layout.add_widget(self._bday)
        layout.add_widget(self._anniv)
        layout.add_widget(self._props)

        layout.add_widget(Divider())

        # This will add buttons at the bottom
        layout2 = Layout([1, 1])
        self.add_layout(layout2)
        layout2.add_widget(Button("OK", self._save), 0) # This will save changes
        layout2.add_widget(Button("Cancel", self._cancel), 1)  # This will cancel changes

        self.fix()

    # The below def, When it loads the view, it populate fields with data from the current file. 
    def reset(self):
        super(CardDetV, self).reset()
        filename = self._model.current_file
        path = os.path.join(self._model.card_dir, filename).encode("utf-8")

        # Below it will get data from the C library
        fn = self._model.get_fn(filename)
        bday = self._model.lib.BDAY_File(path)
        anniv = self._model.lib.ANNIV_File(path)
        props_raw = self._model.lib.getOptP(path)
        prop_count = len(props_raw.decode().splitlines()) if props_raw else 0

        # Below it will load values into fields
        self.data = {
            "filename": filename,
            "fn": fn,
            "bday": parse_date_time(bday),
            "anniv": parse_date_time(anniv),
            "props": str(prop_count)
        }

    # The below def will save the updated filename and FN to the disk and database.
    def _save(self):
        self.save()
        old_filename = self._model.current_file
        new_filename = self.data["filename"].strip()
        new_fn = self.data["fn"].strip()

         # The below will make sure the filename is valid
        if not new_filename.endswith(".vcf") or not new_filename:
            self.scene.add_effect(PopUpDialog(self.screen, "Oops, Invalid file name!", ["OK"]))
            return

        try: 
            # So If filename was changed, update it on disk and in DB
            if new_filename != old_filename:
                old_path = os.path.join(self._model.card_dir, old_filename)
                new_path = os.path.join(self._model.card_dir, new_filename)

                if os.path.exists(new_path):
                    self.scene.add_effect(PopUpDialog(self.screen, "Oops, file with new name already exists!", ["OK"]))
                    return

                # Below it will rename the file on disk
                os.rename(old_path, new_path)
                self._model.current_file = new_filename

                # The below will Update all relevenet DB tables 
                conn = self.screen._db_conn
                cursor = conn.cursor()

                cursor.execute("SET FOREIGN_KEY_CHECKS = 0")
                cursor.execute("UPDATE FILE SET filename = %s WHERE filename = %s", (new_filename, old_filename))
                cursor.execute("UPDATE CONTACT SET file_id = %s WHERE file_id = %s", (new_filename, old_filename))
                cursor.execute("UPDATE vcard_data SET filename = %s WHERE filename = %s", (new_filename, old_filename))
                cursor.execute("UPDATE optional_props SET vcard_filename = %s WHERE vcard_filename = %s", (new_filename, old_filename))
                cursor.execute("SET FOREIGN_KEY_CHECKS = 1")
                conn.commit()

            # Below it will always update FN regardless of filename change
            self._model.update_fn(new_filename, new_fn)

            # Below it will show success message
            self.scene.add_effect(PopUpDialog(
                self.screen, 
                "Changes saved successfully!", 
                ["OK"], 
                on_close=lambda _: raise_(NextScene("Main"))
            ))

        except Exception as e: 
            # Below if anything fails, it will show the error
            self.scene.add_effect(PopUpDialog(self.screen, f"Oops, Rename failed: {e}", ["OK"]))


    # The below def will cancel editing and return to the main view.
    @staticmethod
    def _cancel():
        raise NextScene("Main")

# The below class CreV, is the view that lets the user create a new vCard file with a full name.
class CreV(Frame): 
    # The below def initializes the Create New vCard form where the user inputs a file name and full name FN.
    def __init__(self, screen, model):
        super(CreV, self).__init__(screen, screen.height * 2 // 3, screen.width * 2 // 3, hover_focus=True, can_scroll=False, title="Create New vCard")
        self._model = model

        # Below is the layout for text fields
        layout = Layout([100], fill_frame=True)
        self.add_layout(layout)
        layout.add_widget(Text("Plz, Enter File Name:", "filename"))
        layout.add_widget(Text("Plz, Enter Full Name:", "fn"))

        # Below is the layout for buttons
        layout2 = Layout([1, 1])
        self.add_layout(layout2)
        layout2.add_widget(Button("Create", self._save), 0)
        layout2.add_widget(Button("Cancel", self._cancel), 1)
        self.fix()

    # The below def will save handler for Create button. That said, it validates input and creates a vCard using the C library.
    def _save(self):
        self.save()
        filename = self.data["filename"].strip()
        fn = self.data["fn"].strip()

        # Below is the basic input validation
        if not filename.endswith(".vcf") or not filename or not fn:
            self.scene.add_effect(PopUpDialog(self.screen, "Invalid filename or FN", ["OK"]))
            return

        full_path = os.path.join(self._model.card_dir, filename)

        # Below it will check if file already exists
        if os.path.exists(full_path):
            self.scene.add_effect(PopUpDialog(self.screen, "Oops, File already exists!", ["OK"]))
            return

        # Below it will call createCard(char* filename, char* FN) from my C library
        result = self._model.lib.createNewC(ctypes.c_char_p(full_path.encode("utf-8")), ctypes.c_char_p(fn.encode("utf-8")))

        # Below it will show result
        if result != 0:
            self.scene.add_effect(PopUpDialog(self.screen, f"Oops, Failed to create vCard (code {result})", ["OK"]))
        else:
            self._model.load_cards()  # This will refresh card list
            self.scene.add_effect(PopUpDialog(self.screen, "vCard created successfully!", ["OK"], on_close=self._goto_main))

    # The below def will cancel creation and return to the main view.
    @staticmethod
    def _cancel():
        raise NextScene("Main")   
    # Below is the helper to return to the main screen after popup confirmation.
    def _goto_main(self, _):
        raise NextScene("Main")
    # The below def will clear the input fields every time the form is loaded.
    def reset(self):
        super(CreV, self).reset()
        self.data = {
            "filename": "",
            "fn": ""
        }


# The below def will set up and manage all the UI scenes views used in the app.
def demo(screen, scene):
    scenes = [
        Scene([LogV(screen)], -1, name="Login"), # This is for MySQL login screen
        Scene([ListView(screen, cards)], -1, name="Main"), # This is for the Main vCard file list
        Scene([ContactView(screen, cards)], -1, name="Edit"),
        Scene([CreV(screen, cards)], -1, name="Create"),
        Scene([DBQueV(screen)], -1, name="DB Queries"),  # This will run DB queries
        Scene([CardDetV(screen, cards)], -1, name="Details") # This is for the Full card info + rename
    ]
    # Below it will start the screen, and allow it to resize while switching scenes
    screen.play(scenes, stop_on_resize=True, start_scene=scene, allow_int=True)



cards = VCardMan() # This will initialize the vCard manager that talks to C
last_scene = None  # This is used to remember the last active scene if screen is resized

# The below will run the app in a loop, handling screen resizing
while True:
    try:
        Screen.wrapper(demo, catch_interrupt=True, arguments=[last_scene])
        sys.exit(0)
    except ResizeScreenError as e:
        last_scene = e.scene # This will restore the last scene after resize
