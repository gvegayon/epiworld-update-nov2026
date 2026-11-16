// ModelSEIRQuarantine on two layers at once, households and work, with
// contact tracing in both. In epiworld 0.18 there were two quarantine models,
// one for a network and one for mixing, each with its own hand-written states
// (Quarantined Susceptible, Quarantined Exposed, Isolated Recovered, ...).
// Writes the daily counts by status and stage, with and without the policy
// (10 runs each).
#include "../vendor/epiworld.hpp"
#include <fstream>

using namespace epiworld;

int main()
{

    const size_t n = 20000u;
    std::ofstream out("results/quarantine.csv");
    out << "policy,run,date,status,stage,count\n";

    for (bool policy : {false, true})
    {

        epimodels::ModelSEIRQuarantine model(
            "Flu", n,
            0.001,             // prevalence
            0.06,              // transmission probability
            3.0,               // latent period
            1.0 / 7.0,         // recovery rate
            0.01,              // hospitalization rate
            7.0,               // hospitalization period
            policy ? 5.0 : -1, // detection delay (negative: never)
            14,                // quarantine period
            0.5,               // quarantine willingness
            0.9,               // isolation willingness
            7,                 // isolation period (from onset)
            0.5,               // contact tracing probability
            4u                 // contact tracing window
        );
        model.verbose_off();

        std::vector< size_t > household(n);
        for (size_t i = 0u; i < n; ++i)
            household[i] = i / 4u;
        auto home = layer::mixing("household");
        home->set_groups(assign::labels(household), n / 4u);
        home->set_contact_prob({0.3});
        model.add_layer(home);
        model.add_layer(layer::network("work", graph::smallworld(8, 0.05)));

        for (int run = 0; run < 10; ++run)
        {
            model.run(250, 70 + run);

            std::vector< int > date, counts;
            std::vector< std::string > status, stage;
            model.get_db().get_hist_total(&date, &status, &stage, &counts);
            for (size_t i = 0u; i < date.size(); ++i)
                out << (policy ? "detection + tracing" : "no policy") << ","
                    << run << "," << date[i] << "," << status[i] << ","
                    << stage[i] << "," << counts[i] << "\n";
        }

    }

    return 0;

}
