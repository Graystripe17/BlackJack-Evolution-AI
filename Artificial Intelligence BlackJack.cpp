/*
Artificial Intelligence BlackJack
Discover the best strategies in BlackJack by pitting computers against each other. Analyze hit-percentages, betting patterns, and fold thresholds. Winners of a generation will reproduce a new generation with a similar "genetic makeup" strategy.
Created by Script_Kitty
8-16-2015
*/

#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<algorithm>
#include<deque>
#include<stack>
#include<vector>
#include<time.h>
#include<random>
#include<string>
#include<map>
#include<windows.h>
#include<conio.h>
#include<random>
#define heart 0;
#define club 1;
#define diamond 2;
#define spade 4;
#define TWICE for(int i = 0; i < 2; i++)
using namespace std;

const int init_bet = 10;

random_device rd;
mt19937 mt(rd());
uniform_real_distribution<double> dist(0, 1);


class card;
class players;
float rand_dec();
void shuffle_cards();
void hit_once(vector<players>::iterator whoHit);
void hitting_round();
void betting_round();
enum vals {J=10, Q=10, K=10, A=11};
class card {
	public:
		int value;
		int suit;
		card(int value, int suit){
			this->value = value;
			this->suit = suit;
			// _notation is better
		}
};
class players {
	public:
		string name;
		vector<card> current_hand;
		bool fold;
		int sum;
		int balance;
		int round_contribution;
		// [0, 1] for each value 12-20
		map<int, float> hit_chance;
		// How much are you willing to call if you have x hand?
		map<int, int> max_call;
		// How much do you plan to raise to for x hand?
		map<int, int> raise;
		players(string passed_name) {
			balance = 1000;
			sum = 0;
			round_contribution = 0;
			name = passed_name;
			fold = false;
			for(int nine = 12; nine < 21; nine++) {
				// For each of the 9 values, set in random values
				hit_chance[nine] = rand_dec();
				raise[nine] = rand() % 30;
				max_call[nine] = raise[nine] + rand() % 50;
			}
		}
};
// Each inline member of a template class must be prefaced with the template parameter list.
// template<class card>
// Alternatively forward declare your classes
deque<card> cards_;
vector<players> workerVec;

// The following are orders
// [12, 13, 14, 15, 16, 17, 18, 19, 20]

// Returns a float from 0.0 - 1.0
float rand_dec() {
	return dist(mt);
}
void shuffle_cards() {
	// Check for clearing
	cards_.clear();
	// Instantiate Deck
	for(int thirteen = 1; thirteen < 14; thirteen++) {
		for(int four = 0; four < 4; four++) {
			card birthCard(thirteen, four);
			switch(birthCard.value) {
				case 11:
					birthCard.value = J;
					break;
				case 12:
					birthCard.value = Q;
					break;
				case 13:
					birthCard.value = K;
					break;
				case 1:
					birthCard.value = A;
					break;
				default:
					// The card was 2-9
					break;
			}

			cards_.push_back(birthCard);
		}
	}
	random_shuffle(cards_.begin(), cards_.end());
	// (cards_.begin(), cards_.end(), mt);
}

void hit_once(vector<players>::iterator whoHit) {
	// Update current hand with front card
	whoHit->current_hand.push_back(cards_.front());
	// Update sum
	whoHit->sum += whoHit->current_hand.back().value;
	// Destroy front card
	cards_.pop_front();
}

void hitting_round() {
	for(auto targetWorker = workerVec.begin(); targetWorker != workerVec.end(); ++targetWorker) {
		while(targetWorker->sum <= 11) {
			// Definitely Hit
			hit_once(targetWorker);
		} 
		targetWorker->fold = false;
		while(rand_dec() <= targetWorker->hit_chance[targetWorker->sum]) {
			if(targetWorker->sum == 21){
				// Definitely Stay (could use break)
				break;
			} else {
				hit_once(targetWorker);
				if(targetWorker->sum > 21) {
					bool ace_to_one = false;
					for(auto acer = targetWorker->current_hand.begin(); acer != targetWorker->current_hand.end(); ++acer) {
						if(acer->value == 11) {
							acer->value = 1;
							targetWorker->sum -= 10;
							ace_to_one = true;
						}
					}
					// No change
					if(!ace_to_one) {
						targetWorker->fold = true;
						
						cout<<targetWorker->name<<" busted with "<<targetWorker->sum<<"\n[";
						for (int i = 0; i < targetWorker->current_hand.size(); i++) {
							cout<<targetWorker->current_hand[i].value<<".";
						}
						cout<<"]\n\n";
						break;
					}
				}
				if(targetWorker->sum == 21) {
					break;
				}
			}
		}
	
		if(targetWorker->fold == false) {
			cout<<targetWorker->name<<" started with "<<targetWorker->sum<<"\n[";
			for (int i = 0; i < targetWorker->current_hand.size(); i++) {
					cout<<targetWorker->current_hand[i].value<<".";
			}
			cout<<"]\n\n";
		}
	}
}



void betting_round() {
	cout<<"\nSTART BETTING\n";

	int total_pot = 0;
	int required_bet = init_bet;
	// Initial betting
	for(auto focus = workerVec.begin(); focus != workerVec.end(); ++focus) {
		focus->balance -= init_bet;
		total_pot += init_bet;
		focus->round_contribution = init_bet;
	}


	// Who wishes to raise
	// Circle back until we hit the last raiser
	string raiser_name = "call";
	auto focus_bet = workerVec.begin();

	while(!(raiser_name == focus_bet->name || (raiser_name == "call" && focus_bet == workerVec.end() - 1))) {
		if(!focus_bet->fold) {
			// If your current raise rate is higher than what's on the table
			if(focus_bet->raise[focus_bet->sum] > required_bet) {
				// Lower your balance by the change
				focus_bet->balance -= focus_bet->raise[focus_bet->sum] - required_bet;
				// Add the change to the pot
				total_pot += focus_bet->raise[focus_bet->sum] - required_bet;
				// Change the required bet to what you just announced
				required_bet = focus_bet->raise[focus_bet->sum];
				// Update how much you added to the pot
				focus_bet->round_contribution = focus_bet->raise[focus_bet->sum];
				// Update last raiser
				raiser_name = focus_bet->name;

				cout<<focus_bet->name<<" raised to "<<required_bet<<"; total pot: "<<total_pot<<"\n";
			} else {
				// Confident you won't raise; call or fold?
				// If the current raise rate is higher than your calling 
				if(required_bet > focus_bet->max_call[focus_bet->sum] && focus_bet->sum != 21) {
					// Fold
					focus_bet->fold = true;
					// Though folded, simulate a call
					focus_bet->round_contribution = required_bet;
					cout<<focus_bet->name<<" folded.\n";
					break;
				} else {
					// Will call
					focus_bet->balance -= focus_bet->max_call[focus_bet->sum] - focus_bet->round_contribution;
					total_pot += focus_bet->max_call[focus_bet->sum] - focus_bet->round_contribution;
					// Required bet stays the same
					// Update round_contribution as amount called
					focus_bet->round_contribution = focus_bet->max_call[focus_bet->sum];
					
					cout<<focus_bet->name<<" called the required "<<required_bet<<"\n";
				}

			}
		}
		if(focus_bet != workerVec.end() - 1) {
			focus_bet++;
		} else {
			focus_bet = workerVec.begin();
		}
	}

	// Show Hand

	players* winner;
	// Now create a dummy
	players ghost_score_one("Ghost");
	// The score to beat if not bust
	ghost_score_one.sum = 1;
	// Set winner as ghost as temp
	winner = &ghost_score_one;
	vector<players *> high_tie;
	for(auto revealer = workerVec.begin(); revealer != workerVec.end(); ++revealer) {
		if(!revealer->fold) {
			if(revealer->sum > winner->sum) {
				high_tie.clear();
				// Replace the winner with revealer
				// Convert the Iterator into a pointer
				// In vector, grab address and assign to winner
				winner = &(*revealer);
				high_tie.push_back(&(*revealer));
			} else if (revealer->sum == winner->sum) {
				// We have a tie, don't flush
				high_tie.push_back(&(*revealer));
			}
		}
	}

	// Payout
	if(high_tie.size() == 0) {
		// Everybody busted: return what you paid
		for(auto rain_check = high_tie.begin(); rain_check != high_tie.end(); ++rain_check) {
			(*rain_check)->balance += (*rain_check)->round_contribution;
		}
		cout<<"Everyone busted. Rain check delivered.\n";
	} else {
		for(auto tied_champ = high_tie.begin(); tied_champ != high_tie.end(); ++tied_champ) {
			(*tied_champ)->balance += total_pot / high_tie.size();
			
			cout<<(*tied_champ)->name<<" won +"<<total_pot / high_tie.size()<<". Balance: "<<(*tied_champ)->balance<<"\n";
		}
	}
}


void main() {
	

	// Create 6 players
	// You must use pointers; push_back(Jim) is not correct
	players Jim = players("Jim    ");
	players Kevin = players("Kevin  ");
	players Michael = players("Michael");
	players Dwight = players("Dwight ");
	players Pam = players("Pam    ");
	players Kelly = players("Kelly  ");

	workerVec.push_back(Jim);
	workerVec.push_back(Kevin);
	workerVec.push_back(Michael);
	workerVec.push_back(Dwight);
	workerVec.push_back(Pam);
	workerVec.push_back(Kelly);

	for(int gen=1; gen <= 300; gen++) {
		for(int round=1; round <= 200; round++) {
			shuffle_cards();
			// Deal
			// Iterate over workers
		
			cout<<"\n\n\nGENERATION "<<gen<<" | ROUND "<<round<<"\n";

			for(auto clear = workerVec.begin(); clear != workerVec.end(); ++clear) {
				clear->fold = false;
				clear->current_hand.clear();
				clear->sum = 0;
				clear->round_contribution = init_bet;
			}

			TWICE {
				for(unsigned int i = 0; i < workerVec.size(); i++) {
					// Passing in an iterator, not a pointer
					hit_once(workerVec.begin() + i);
					// ^ Might be workerVec.begin() + 1
					// Could just use ::iterator
				}
			}
			hitting_round();


			betting_round();
			// Sleep(20000);
			// string next;
			// getline(cin, next);
		}

		cout<<"\n"<<"End of Gen "<<gen<<"\n";

		players minimum_ghost("Min Ghost");
		minimum_ghost.balance = INT_MIN;
		players* first_place = &minimum_ghost;
		players* second_place = &minimum_ghost;

		// Who won?
		for(auto announce = workerVec.begin(); announce != workerVec.end(); ++announce) {
			cout<<announce->name<<" ended with "<<announce->balance<<"\n";
			if(announce->balance > first_place->balance) {
				second_place = first_place;
				first_place = &(*announce);
			}
		}

		// Start next breed
		for(auto fresh = workerVec.begin(); fresh != workerVec.end(); ++fresh) {
			// Reformation
			for(int new_hit_chance = 12; new_hit_chance < 21; new_hit_chance++) {
				// Which is upperbound, which is lowerbound?
				uniform_real_distribution<double> hit_dist(
					min(first_place->hit_chance[new_hit_chance], second_place->hit_chance[new_hit_chance]),
					max(first_place->hit_chance[new_hit_chance], second_place->hit_chance[new_hit_chance])
				);
				// Generate a new hit chance randomly between these two values
				fresh->hit_chance[new_hit_chance] = hit_dist(mt);
			}

			for(int new_raise_limit = 12; new_raise_limit < 21; new_raise_limit++) {
				// Which is upperbound, which is lowerbound?
				uniform_real_distribution<double> raise_dist(
					min(first_place->raise[new_raise_limit], second_place->raise[new_raise_limit]),
					max(first_place->raise[new_raise_limit], second_place->raise[new_raise_limit])
				);
				// Generate a new raise limit randomly between these two values
				fresh->raise[new_raise_limit] = raise_dist(mt);
			}

			for(int new_max_call = 12; new_max_call < 21; new_max_call++) {
				// Which is upperbound, which is lowerbound?
				uniform_real_distribution<double> max_call_dist(
					min(first_place->max_call[new_max_call], second_place->max_call[new_max_call]),
					max(first_place->max_call[new_max_call], second_place->max_call[new_max_call])
				);
				// Generate a new max call randomly between these two values
				fresh->max_call[new_max_call] = max_call_dist(mt);
			}

			// Clear out balance
			fresh->balance = 1000;
		}
		
		// Print out the stats of the first_place winner
		cout<<"WINNER STATS\n";
		for(int win = 12; win < 21; win++) {
			cout<<"\n";
			cout<<"Hit Chance "<<win<<": "<<first_place->hit_chance[win]<<"\n";
			cout<<"Max Call   "<<win<<": "<<first_place->max_call[win]<<"\n";
			cout<<"Raise      "<<win<<": "<<first_place->raise[win]<<"\n";
		}

		// string next;
		// getline(cin, next);
	}

	return;
}
