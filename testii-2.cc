#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>

using namespace std;

#define NUM_THREADS 101

//threadien valiseen kommunikointiin
int autoja = NUM_THREADS-1;
int autoja_jonossa = 0;
int autoja_lautalla = 0;

//lastaus
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
//purku
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;

//tulostus+"liikennevalot"
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
//lastaus
pthread_mutex_t mutexL1 = PTHREAD_MUTEX_INITIALIZER;
//purku
pthread_mutex_t mutexL2 = PTHREAD_MUTEX_INITIALIZER;

void *Kaara(void *threadid) {
  long tid = 0;
  long rc = 0;
  char alkupiste = rand() % 2 + 'A';
  char loppupiste = rand() % 2 + 'C';
  tid = (long)threadid;
  rc = pthread_mutex_lock(&mutex1);
  cerr << "Auto luotu " << tid << ", reitti " << alkupiste << "->" 
	<< loppupiste << endl;
  rc = pthread_mutex_unlock(&mutex1);
  rc = pthread_mutex_lock(&mutex1);
  cerr << "Auto " << tid << ", saapui liikennevaloihin" << endl;
  rc = pthread_mutex_unlock(&mutex1);
  pthread_mutex_lock( &mutex1);
  cerr << "Auto " << tid << ", risteysalueella" << endl;
  cerr << "Auto " << tid << ", poistui risteysalueelta" << endl;
  pthread_mutex_unlock( &mutex1);
  if(loppupiste == 'C') {
    pthread_mutex_lock(&mutex1);
    cerr << "Auto " << tid << ", poistuu " << loppupiste << endl;
    pthread_mutex_unlock(&mutex1);
  }else{
    //siirrytaan jonoon
    pthread_mutex_lock(&mutex1);
    ++autoja_jonossa;
    cerr << "Auto " << tid << ", jonottaa lautalle" << endl;
    pthread_mutex_unlock(&mutex1);
    
    //odotetaan lastausta
    pthread_mutex_lock(&mutexL1);
    pthread_cond_wait(&cond, &mutexL1); 
    pthread_mutex_unlock(&mutexL1);
    
    pthread_mutex_lock(&mutex1);
    cerr << "Auto " << tid << ", nousee lautalle" << endl;
    --autoja_jonossa;
    ++autoja_lautalla;
    pthread_mutex_unlock(&mutex1);
    
     //odotetaan purkua
    pthread_mutex_lock(&mutexL2);
    pthread_cond_wait(&cond2, &mutexL2);
    pthread_mutex_unlock(&mutexL2);
    
    //auto poistuu lautalta ja kartalta
    pthread_mutex_lock(&mutex1);
    cerr << "Auto " << tid << ", poistuu "<< loppupiste << endl;
    --autoja_lautalla;  
    pthread_mutex_unlock(&mutex1);
  }
  pthread_mutex_lock(&mutex1);
  --autoja; //auto poistuu ohjelmasta
  pthread_mutex_unlock(&mutex1);
  pthread_exit(NULL);
}

void *Lautta(void *threadid) {
  //arvotaan lahtopaikka
  char paikat[] = "LD";
  char paikka = paikat[rand() % 2];

  //suora kopio, siistii ajotusta hieman
  int milisec = 4; // length of time to sleep, in miliseconds
  struct timespec req = {0};
  req.tv_sec = 0;
  req.tv_nsec = milisec * 1000000L;
  
  while(true){
    nanosleep(&req, (struct timespec *)NULL);
    if(autoja<1) break;   //autoja ei enaa ole, lopetetaan
    if(paikka == 'L'){ //lastaus L:ssä
      
      pthread_mutex_lock(&mutex1);
      cerr << "Lautta saapuu "<< paikka << endl;
      pthread_mutex_unlock(&mutex1);
      
      while(autoja_jonossa<1){ //ei siirreta lauttaa ilman autoja
	nanosleep(&req, (struct timespec *)NULL);
      }
      while(autoja_jonossa>0){ //lastataan kaikki autot
	pthread_cond_broadcast(&cond); 
      if(autoja_jonossa<1) break;
      }
      paikka = 'D'; //siirtyminen
    }else{ //purku D:ssa
      pthread_mutex_lock(&mutex1);
      cerr << "Lautta saapuu " << paikka << endl;
      pthread_mutex_unlock(&mutex1);
      
      while(autoja_lautalla>0){ //puretaan kaikki autot
      pthread_cond_broadcast(&cond2);
      }
      paikka = 'L';
    }
  }
  pthread_exit(NULL);
}

int main () {
   pthread_t threads[NUM_THREADS];
   int rc = 0;
   int lautta = 0;
   int i = 0;

   //suora kopio, siistii ajotusta hieman
   int milisec = 2; // length of time to sleep, in miliseconds
   struct timespec req = {0};
   req.tv_sec = 0;
   req.tv_nsec = milisec * 1000000L;
   
   //luodaan lautta
   lautta = pthread_create(&threads[0], NULL, Lautta, (void *) 0);
   if (lautta){
     cerr << "Virhe: threadin luonti epaonnistui, poistutaan" << rc << endl;
     exit(-1);
   }
   
   //luodaan autot
   for( i=1; i < NUM_THREADS; i++ ){
     //tasoitetaan autojen lahtoaikoja sleepilla
     nanosleep(&req, (struct timespec *)NULL);
     rc = pthread_create(&threads[i], NULL, Kaara, (void *)i);
     if (rc){
       cerr << "Virhe: threadin luonti epaonnistui, poistutaan" << rc << endl;
       exit(-1);
     }
   }
   pthread_exit(NULL);
}
