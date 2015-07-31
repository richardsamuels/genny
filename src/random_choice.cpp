#include <stdio.h>
#include <stdlib.h>
#include "random_choice.hpp"
#include "parse_util.hpp"
#include <random>

namespace mwg {
random_choice::random_choice(YAML::Node& ynode) {
    // need to set the name
    // these should be made into exceptions
    // should be a map, with type = random_choice
    if (!ynode) {
        cerr << "Random_Choice constructor and !ynode" << endl;
        exit(EXIT_FAILURE);
    }
    if (!ynode.IsMap()) {
        cerr << "Not map in random_choice type initializer" << endl;
        exit(EXIT_FAILURE);
    }
    if (ynode["type"].Scalar() != "random_choice") {
        cerr << "Random_Choice constructor but yaml entry doesn't have type == "
                "random_choice" << endl;
        exit(EXIT_FAILURE);
    }
    name = ynode["name"].Scalar();
    if (!ynode["next"].IsMap()) {
        cerr << "Random_Choice constructor but next isn't a map" << endl;
        exit(EXIT_FAILURE);
    }

    total = 0;  // figure out how much the next state rates add up to
    cout << "In random choice constructor. About to go through next" << endl;
    for (auto entry : ynode["next"]) {
        cout << "Next state: " << entry.first.Scalar() << " probability: " << entry.second.Scalar()
             << endl;
        vectornodestring.push_back(
            pair<string, double>(entry.first.Scalar(), entry.second.as<double>()));
        total += entry.second.as<double>();
    }
    cout << "Setting nextName to first entry" << endl;
    nextName = vectornodestring[0].first;
    cout << "NextName: " << nextName << endl;
}

void random_choice::setNextNode(unordered_map<string, shared_ptr<node>>& nodes) {
    double partial = 0;  // put in distribution
    cout << "Setting next nodes in random choice" << endl;
    for (auto nextstate : vectornodestring) {
        partial += (nextstate.second / total);
        vectornodes.push_back(pair<shared_ptr<node>, double>(nodes[nextstate.first], partial));
    }
    cout << "Set next nodes in random choice" << endl;
}

// Execute the node
void random_choice::executeNode(shared_ptr<threadState> myState) {
    // pick a random number, and pick the next state based on it.
    double random_number = 0.6;  // Using 0.6 until wiring through the random number generator
    uniform_real_distribution<double> distribution(0.0, 1.0);
    random_number = distribution(myState->rng);
    cout << "random_choice.execute. Random_number is " << random_number;
    for (auto nextstate : vectornodes) {
        if (nextstate.second > random_number) {
            // execute this one
            if (stopped)  // short circuit and return if stopped flag set
                return;
            cout << " Next state is " << nextstate.first->name << endl;
            shared_ptr<node> me = myState->currentNode;
            myState->currentNode = nextstate.first;
            return (nextstate.first->executeNode(myState));
        }
    }
}
}
