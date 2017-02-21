
struct PanelMeasurements {
  float voltage;
  float current;
  float power; 
};

queue<int>pastLanterns;

PanelMeasurements prevMeasurements = {0,0,0};
int activeLanterns = 0;

void addLantern() {
  activeLanterns ++; 
}
void removeLantern() {
  activeLanterns --;
}

void printMeasurements(PanelMeasurements measurements) {
  cout << activeLanterns << " "<< measurements.voltage << " " << measurements.current << " " << measurements.power << endl;
}

float shortCircuitCurrentPerIllum[5] = {1.1, 4, 5, 6.2, 6.2}; 
PanelMeasurements measureInputs(int radiance) {
  PanelMeasurements measurements;
  measurements.current = 0.5 * activeLanterns; 
  measurements.voltage = 40.0 * sqrt(1.0 - measurements.current * measurements.current 
                        / (shortCircuitCurrentPerIllum[radiance] * shortCircuitCurrentPerIllum[radiance]) ); 
  if(isnan(measurements.voltage)){
    measurements.voltage = 0;
  }
  measurements.power = measurements.voltage * measurements.current;
  
  return measurements;
}


const float VOLTAGE_LIMT = 33; 

void perturbAndObserve(int radiance) {
  PanelMeasurements newMeasurements = measureInputs(radiance);
  if (newMeasurements.power > prevMeasurements.power ) {
      if (newMeasurements.current > prevMeasurements.current && newMeasurements.voltage > VOLTAGE_LIMT) {
       addLantern(); 
      } else {
       removeLantern();
      }
  } else{
    if (newMeasurements.current > prevMeasurements.current) {
      removeLantern();
    } else if(newMeasurements.voltage > VOLTAGE_LIMT) {
      addLantern();
    }
  }
  pastLanterns.push(activeLanterns);
  prevMeasurements = newMeasurements;

}

bool isOptimal() {
  int total = 0;

  queue<int>Q; 

  while(!pastLanterns.empty()) {
    total += pastLanterns.front(); 
    Q.push(pastLanterns.front());
    pastLanterns.pop();
  }
  while(!Q.empty()) {
    total += Q.front(); 
    pastLanterns.push(Q.front());
    Q.pop();
  }
  return (total - pastLanterns.front()) == (total - pastLanterns.back()) ; 
}


const int DELAY_TIME = 60000; // 1 minute?
enum STATE {SLEEP, SEARCH, OPTIMAL};
STATE state = SLEEP; 

void MPPT(int radiance) {
  PanelMeasurements newMeasurements = measureInputs(radiance);

  // cout << state << " " << pastLanterns.size() << " ";
  printMeasurements(newMeasurements);  

  switch(state) {
    case SLEEP: 
      // delay(DELAY_TIME);
      state = SEARCH;
      break; 
    
    case SEARCH: 
      if(newMeasurements.power == 0 && activeLanterns == 0) {
        // Case wherein there is nothing plugged in... 
        addLantern(); 
        prevMeasurements = newMeasurements;
        newMeasurements = measureInputs(radiance);

        if(newMeasurements.power == prevMeasurements.power) {
          removeLantern();
          state = SLEEP;
        }

      } else if(newMeasurements.power == 0) {
        activeLanterns /= 2;
      } else {         
         if(pastLanterns.size() == 11 && isOptimal()) {
            state = OPTIMAL;
         }
         perturbAndObserve(radiance);
      } 
      break;

    case OPTIMAL: 
      if(pastLanterns.size()!= 11 && newMeasurements.power != prevMeasurements.power) {
        state = SEARCH;
      }
      queue<int>().swap(pastLanterns);
      pastLanterns.push(activeLanterns);
      break;

  }
  if(pastLanterns.size() > 11) {
    pastLanterns.pop();
  }
  prevMeasurements = newMeasurements;

}


int main() {
   cout << "Running test Simulation \n";

   for(int i = 0; i<30; i++) {
      MPPT(3); 
   }
   cout << endl;
   for(int i = 0; i < 20; i++){
      MPPT(1); 
   }
   cout << endl;
   for(int i = 0; i < 20; i++){
      MPPT(3); 
   }
}





