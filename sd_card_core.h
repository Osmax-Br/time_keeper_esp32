/*
#include <IRremote.h>

const int RECV_PIN = 7;
IRrecv irrecv(RECV_PIN);
decode_results results;

void setup(){
  Serial.begin(9600);
  irrecv.enableIRIn();
  irrecv.blink13(true);
}

void loop(){
  if (irrecv.decode(&results)){
        Serial.println(results.value, HEX);
        irrecv.resume();
  }
}
*/
////////////////////////////////////
//sd file system funcions//
////////////////////////////////////
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}
int readFile(fs::FS &fs, const char * path,int x){
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if(!file){
    Serial.println("Failed to open file for reading");
    x = 0;
    return 0;
  }
  file.close();
  return 1;}

  int appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    return 0;
  }
  if(file.print(message)){
    Serial.println("Message appended");
    file.close();
    return 1;
  } else {
    Serial.println("Append failed");
    file.close();
    return 0;
  }
  
}

///////////////////////////////
//sqlite3 functions//
//////////////////////////////
const char* data = "Callback function called";
static int callback(void *data, int argc, char **argv, char **azColName){
   int i;
   Serial.printf("%s: ", (const char*)data);
   for (i = 0; i<argc; i++){
       Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   Serial.printf("\n");
   return 0;
}

int openDb(const char *filename, sqlite3 **db) {
   int rc = sqlite3_open(filename, db);
   if (rc) {
       Serial.printf("Can't open database: %s\n", sqlite3_errmsg(*db));
       return rc;
   } else {
       Serial.printf("Opened database successfully\n");
   }
   return rc;
}
char *zErrMsg = 0;
int db_exec(sqlite3 *db, const char *sql) {
   Serial.println(sql);
   long start = micros();
   int rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
   if (rc != SQLITE_OK) {
       Serial.printf("SQL error: %s\n", zErrMsg);
       sqlite3_free(zErrMsg);
   } else {
       Serial.printf("Operation done successfully\n");
   }
   Serial.print(F("Time taken:"));
   Serial.println(micros()-start);
   return rc;
}
/*
//void setup sd card init
SPIClass sd_spi(HSPI);
  sd_spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, sd_spi))
    Serial.println("SD Card: mounting failed.");
  else{
    Serial.println("SD Card: mounted.");
  sqlite3 *db1;
  char *zErrMsg = 0;
  int rc;
  int x = readFile(SD,"/times.db",x);
  if(x==0){
    Serial.println("not here");
    writeFile(SD, "/times.db","");
    rc = db_exec(db1, database_init);
    if (rc != SQLITE_OK)
  {
    sqlite3_close(db1);
    return;
  }}
  else{
    Serial.println("here");
    }}
*/


