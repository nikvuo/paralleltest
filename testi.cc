#include <iostream>
#include <cstdlib>
#include <pthread.h>

using namespace std;

#define NUM_THREADS     101

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

void *Kaara(void *threadid)
{
   long tid;
   char alkupiste = rand() % 2 + 'A';
   char loppupiste = rand() % 2 + 'C';
   tid = (long)threadid;
   pthread_mutex_lock(&mutex1);
   cerr << "Auto luotu " << tid << ", reitti " << alkupiste << "->" << loppupiste << endl;
   pthread_mutex_unlock(&mutex1);
   pthread_mutex_lock(&mutex1);
   cerr << "Auto " << tid << " saapui liikennevaloihin" << endl;
   pthread_mutex_unlock(&mutex1);
   pthread_mutex_lock( &mutex1);
   cerr << "Auto " << tid << " risteysalueella" << endl;
   cerr << "Auto " << tid << ", poistui risteysalueelta" << endl;
   pthread_mutex_unlock( &mutex1);
   if(loppupiste == 'C') {
     pthread_mutex_lock(&mutex1);
     cerr << "Auto " << tid << " poistuu " << loppupiste << endl;
     pthread_mutex_unlock(&mutex1);
   }
   else {
     pthread_mutex_lock(&mutex1);
     cerr << "Auto " << tid << ", jonottaa lautalle" << endl;
     pthread_mutex_unlock(&mutex1);

     //pthread_join(&threads[100], NULL); Pitäisi pysäyttää threadi ja odottaa lauttaa
   }
   pthread_exit(NULL);
}

void *Lautta(void *threadid)
{
  char paikat[] = "LD";
  char paikka = paikat[rand() & 2];
  unsigned int i = 0;
  for(i=0; i < 100 ; i++)
  {
    cout << "Lautta saapuu " << paikka << endl;
    if(paikka == 'L') {
      paikka = 'D';
	}
    else {
      paikka = 'L';
    }
  }
}

int main ()
{
   pthread_t threads[NUM_THREADS];
   int rc;
   int i;
   for( i=0; i < NUM_THREADS-1; i++ ){
     //cout << "Auto, " << i << endl;
      rc = pthread_create(&threads[i], NULL, Kaara, (void *)i);
      if (rc){
         cerr << "Error:unable to create thread," << rc << endl;
         exit(-1);
      }
   }
   // rc = pthread_create(&threads[100], NULL, Lautta, (void *) 100);
   pthread_exit(NULL);
}
