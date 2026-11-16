// The same SEIR, with the same mean durations (latent 4 days, infectious 7),
// drawn two ways: geometric (the only shape in epiworld 0.18) and lognormal
// or gamma (drawn once, on entering the stage). Writes the daily number of
// infectious agents, for 20 runs of each.
#include "../vendor/epiworld.hpp"
#include <fstream>

using namespace epiworld;

static Virus seir(bool shaped)
{
    Virus v("Flu");
    const int S = v.add_stage("Susceptible");
    const int E = v.add_stage("Exposed", stage::carrier);
    const int I = v.add_stage("Infectious", stage::infectious);
    const int R = v.add_stage("Recovered");
    v.infection(S, E, "Transmission probability");
    if (shaped)
    {
        v.after(E, I, Duration::lognormal(4.0, 1.0));
        v.after(I, R, Duration::gamma(7.0, 2.0));
    }
    else
    {
        v.after(E, I, Duration::geometric(4.0));
        v.rate(I, R, 1.0 / 7.0);
    }
    return v;
}

int main()
{

    std::ofstream out("results/durations.csv");
    out << "durations,run,date,infectious\n";

    for (bool shaped : {false, true})
    {

        Model model(20000u);
        model.verbose_off();
        model.add_param(0.04, "Transmission probability");
        model.add_virus(seir(shaped), assign::count(20));
        model.add_layer(layer::network("contacts", graph::smallworld(10, 0.05)));

        for (int run = 0; run < 20; ++run)
        {
            model.run(365, 100 + run);
            std::vector< int > date, counts;
            std::vector< std::string > status, stage;
            model.get_db().get_hist_total(&date, &status, &stage, &counts);
            for (size_t i = 0u; i < date.size(); ++i)
                if (stage[i] == "Infectious")
                    out << (shaped ? "lognormal / gamma" : "geometric") << ","
                        << run << "," << date[i] << "," << counts[i] << "\n";
        }

    }

    return 0;

}
