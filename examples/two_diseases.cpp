// Two diseases with different natural histories on one population: influenza
// (SEIR, with a second strain) and RSV (SIR, longer infection). At most one
// infection per agent at a time; an agent recovered from one can catch the
// other. A flu vaccine targets influenza only. In epiworld 0.18 every virus
// shared the model's states, so a "recovered" agent was recovered from all.
#include "../vendor/epiworld.hpp"
#include <fstream>

using namespace epiworld;

int main()
{

    const size_t n = 20000u;
    Model model(n);
    model.verbose_off();
    model.add_param(0.07, "Flu transmission");
    model.add_param(0.09, "Flu B transmission");
    model.add_param(0.03, "RSV transmission");
    model.add_layer(layer::network("contacts", graph::smallworld(10, 0.05)));

    // Influenza: SEIR, lognormal latent period
    Virus flu("Influenza");
    const int S = flu.add_stage("Susceptible");
    const int E = flu.add_stage("Exposed", stage::carrier);
    const int I = flu.add_stage("Infectious", stage::infectious);
    const int R = flu.add_stage("Recovered");
    flu.infection(S, E, "Flu transmission");
    flu.after(E, I, Duration::lognormal(2.0, 0.5));
    flu.rate(I, R, 1.0 / 5.0);
    model.add_virus(flu, assign::count(10));

    // A second strain of the same disease, more transmissible
    Virus flu_b = flu.strain("Influenza B");
    flu_b.use_param("Flu transmission", "Flu B transmission");
    model.add_virus(flu_b, assign::count(5));

    // RSV: SIR, about ten days infectious; a different disease
    Virus rsv("RSV");
    const int rS = rsv.add_stage("Susceptible");
    const int rI = rsv.add_stage("Infectious", stage::infectious);
    const int rR = rsv.add_stage("Recovered");
    rsv.infection(rS, rI, "RSV transmission");
    rsv.rate(rI, rR, 1.0 / 10.0);
    model.add_virus(rsv, assign::count(10));

    // A flu vaccine: tools name the diseases they act on
    Tool vax("Flu vaccine");
    vax.set_susceptibility_reduction(0.6);
    vax.add_target("Influenza");
    model.add_tool(vax, assign::share(0.3));

    model.run(250, 15);

    // The diagram, drawn from the declarations (both diseases and statuses)
    std::ofstream("results/two_diseases.mmd") << model.draw_natural_history();

    // Each disease's counts (a disease column in every output)
    std::ofstream out("results/two_diseases.csv");
    out << "disease,date,stage,count\n";
    std::vector< int > date, counts;
    std::vector< std::string > status, stage, disease;
    model.get_db().get_hist_total(&date, &status, &stage, &counts, all_diseases, &disease);
    for (size_t i = 0u; i < date.size(); ++i)
        out << disease[i] << "," << date[i] << "," << stage[i] << ","
            << counts[i] << "\n";

    // New infections by strain
    std::ofstream tout("results/two_diseases_strains.csv");
    tout << "virus,date\n";
    std::vector< int > tdate, source, target, virus, expo, layer_id;
    model.get_db().get_transmissions(tdate, source, target, virus, expo, layer_id);
    for (size_t i = 0u; i < tdate.size(); ++i)
        if (source[i] >= 0)
            tout << model.get_virus(static_cast< size_t >(virus[i])).get_name()
                 << "," << tdate[i] << "\n";

    // Agents who had both diseases (one after the other)
    int both = 0;
    for (size_t i = 0u; i < n; ++i)
        both += ((model.get_episode(i, 0) > 0) && (model.get_episode(i, 1) > 0)) ? 1 : 0;
    std::ofstream sout("results/two_diseases_summary.csv");
    sout << "both\n" << both << "\n";

    return 0;

}
