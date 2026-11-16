// One SEIR model, three contact layers: households (mixing within groups),
// work (a small-world network) and public spaces (a contact matrix across
// three age groups). Not possible in epiworld 0.18: a model had either a
// network or mixing by entities, never both. Writes the daily transmissions
// by layer.
#include "../vendor/epiworld.hpp"
#include <fstream>
#include <map>

using namespace epiworld;

int main()
{

    const size_t n = 20000u;
    epimodels::ModelSEIR model("Flu", n, 0.001, 0.02, 4.0, 1.0 / 7.0);
    model.verbose_off();

    // Households of four: each pair of housemates meets with probability 0.5
    std::vector< size_t > household(n);
    for (size_t i = 0u; i < n; ++i)
        household[i] = i / 4u;
    auto home = layer::mixing("household");
    home->set_groups(assign::labels(household), n / 4u);
    home->set_contact_prob({0.5});
    model.add_layer(home);

    // Work: the same coworkers every day
    model.add_layer(layer::network("work", graph::smallworld(10, 0.05)));

    // Public spaces: strangers, by age group
    auto pub = layer::mixing("public");
    pub->set_groups(assign::shares({{0, 0.25}, {1, 0.5}, {2, 0.25}}), 3u);
    pub->set_contact_matrix({
        0.6, 0.3, 0.1,
        0.2, 0.6, 0.2,
        0.1, 0.4, 0.5
    });
    model.add_param(4.0, "Public contact rate");
    pub->set_contact_rate("Public contact rate");
    model.add_layer(pub);

    model.run(150, 2131);

    std::vector< int > date, source, target, virus, expo, layer_id;
    model.get_db().get_transmissions(date, source, target, virus, expo, layer_id);

    std::map< std::pair< int, int >, int > counts;
    for (size_t i = 0u; i < date.size(); ++i)
        if (layer_id[i] >= 0)
            ++counts[{date[i], layer_id[i]}];

    std::ofstream out("results/layers.csv");
    out << "date,layer,n\n";
    for (const auto & [key, x] : counts)
        out << key.first << "," << model.get_layer(key.second).get_name()
            << "," << x << "\n";

    return 0;

}
