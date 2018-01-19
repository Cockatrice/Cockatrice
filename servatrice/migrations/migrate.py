import os.path

start = 1;
mysql_config = "migrate";
database = "servatrice";
safe_mode = True;

print("========================================");
print("-- Python Mirgation Tool - By Lachee ---")
print("In order to safely migrate into the database, a mysql config needs to be setup.");
print("Please make sure you have created a temporary one by using the mysql_config_editor.");
print("eg: mysql_config_editor set --login-path=<configname> --host=localhost --user=<username> --password");
print("========================================");
print();

mysql_config = input("--login-path: ")
database = input("database: ");
start = int(input("current schema: "))
do_safemode = input("pause after each migration? y | n: ");
if (do_safemode == 'n'):
    safe_mode = False;

#Does all the big scary things here
print("Migrating from schema version " + str(start) + "...");
current = start;
iterating = True;
while (iterating):

    #Prepare the name
    start_fill = str(current).zfill(4);
    next_fill = str(current + 1).zfill(4);
    filename = "servatrice_" + start_fill + "_to_" + next_fill + ".sql";

    #make sure the file exists
    if (not os.path.exists(filename)):

        #We havn't found the file, we must have reached the end
        print("Failed to locate next migration: " + filename);
        iterating=False;
        
    else:

        #We located the file, Migrate it 
        print("Migrating: " + filename);

        #Prepare the command and execute the command
        command = "mysql --login-path="+mysql_config+" " + database + " < " + filename
        os.system(command);

        #check if we want to continue
        do_continue = "y"
        if (safe_mode):
            do_continue = input("Continue Migration? y | n: ")

        #Continue if we want too
        if (do_continue != "y"):

            #We do not wish to continue, terminate
            iterating=False
            
        else:

            #We wish to continue, so increment teh counter.
            #if we have meet the max, stop.
            current += 1
            if (current > 99999):
                iterating=False;

    #Print a new line to help seperate things
    print();

#We have finished migrating
print("Finished Migrating. Completed " + str(start) + " to " + str(current));
