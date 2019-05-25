This repository moved to GitLab: https://gitlab.com/ToKe79/atlantistalker

# atlantistalker
Atlantis Talker is a talker based on NUTS. The talker runs as a service on a host machine and users can connect to the service using telnet protocol (e.g. `telnet example.com 7000` when the talker is bound to port 7000 on the host machine).

The code in this repository is a copy of the code published on SourceForge as SVN by one of the original coders nicknamed Spartakus.

The code of the Atlantis Talkers, that runs at `telnet talker.sk 7000`, is not public.

## configuration
Copy `config.h.sample` to `config.h` and edit `config.h`. Adjust `DB*` variables and path for temporary files storage. The path can be relative to the executable (in `runtime` folder) or absolute. I recommend using ramdisk for this purpose.

## prepare database
Use your weapon of choice to create a database. Or create database using the command line:
```
$ sudo mysql -u root -h <your_dbhost> -p
> CREATE DATABASE `<your_dbname>`;
> GRANT ALL ON `<your_dbname>`.* to '<your_dbuser>'@'<host_where_talker_runs>' IDENTIFIED BY '<your_dbpass';
> FLUSH PRIVILEGES;
> QUIT
```
When the database and talker are on the same server, use `localhost` as host for both - `<your_dbhost>` and `<host_here_talker_runs>`. Then use the provided database dump to populate the database with data.
```
cat db_dump/dump.sql | mysql <your_dbname> -u <your_dbuser> -h <your_dbhost> -p [your_dbpass]
```

## dependencies
On Debian based systems install these packages:
- libxml2-dev
- default-libmysqlclient-dev
- build-essential
- ccache

## compilation

```
make HAVE_CONFIG=1 DEBUG=1
```

## run
```
cd runtime
./atlantis
```

## connect
From localhost:
```
telnet localhost 7000
```
From other hosts:
```
telnet <yourhostname> 7000
```

Talker uses two ports:
- 7000 - for general use
- 7001 - for "wizzes" login (if they want to login as "invisible" user
