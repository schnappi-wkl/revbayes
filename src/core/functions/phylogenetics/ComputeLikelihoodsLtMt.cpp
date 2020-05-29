#include "ComputeLikelihoodsLtMt.h"

#include <vector>
#include <iostream>
#include <cmath>

#include "RbConstants.h"
#include "RbMathMatrix.h"
#include "MatrixReal.h"
#include "TypedDagNode.h"
#include "Tree.h"
#include "TopologyNode.h"

namespace RevBayesCore {
    class DagNode;
    class MatrixReal; }

using namespace RevBayesCore;

/**
 * Construct the vector containig all branching and sampling times + time points for which we want
 * to compute the density.
 *
 * \param[in]    start_age              Start age of the process.
 * \param[in]    time_points            Times for which we want to compute the density.
 * \param[in]    verbose                If true displays warnings and information messages.
 * \param[in]    occurrence_ages        Vector of occurrence ages.
 * \param[in]    timeTree               Tree for ancestral populations size inference.
 *
 * \return    The matrix of log-Mt values through time.
*/
std::vector<Event> RevBayesCore::PoolEvents(             const TypedDagNode<double> *start_age,
                                                         const std::vector<double> &time_points,
                                                         const std::vector<double> &occurrence_ages,
                                                         bool verbose,
                                                         const Tree &timeTree,
                                                         const std::vector<double> &timeline)
{
    // get node/time variables
    const size_t num_nodes = timeTree.getNumberOfNodes();

    // number of extant lineages
    size_t nb_extant = 0;

    // vector of events
    std::vector<Event>         events;

    // classify nodes
    events.clear();
    events.push_back(Event(start_age->getValue(), "origin"));

    for (size_t i = 0; i < num_nodes; i++)
    {
        const TopologyNode& n = timeTree.getNode( i );

        //isFossil is an optional condition to obtain sampled ancestor node ages
        /*
        Node labels :
        fl = fossil leaf
        b  = "true" bifurcation
        b' = "false" bifurcation (artefact of the sampled ancestors representation)
        sa = sampled ancestor
        el = extant leaf
        . = time points at which density is computed

         __|___             .
        |  b   |
        |      |            .
        fl     |
             b'|___ sa      .
               |
               |            .
               el

         1. Pick a fossil among those with brl > 0 (prob = 1/m)
         2. Set brl = 0
         */

        if ( n.isFossil() && n.isSampledAncestor() )  //isFossil is optional (all sampled ancestors are fossils)
        {
            // node is sampled ancestor
            events.push_back(Event(n.getAge(), "sampled ancestor")) ;

        }
        else if ( n.isFossil() && !n.isSampledAncestor() )
        {
            // node is fossil leaf
            // For now, we assume there is no information/labels on removal
            // @todo add fossil-removed/non-removed
            events.push_back(Event(n.getAge(),"fossil leaf")) ;
            // events.push_back(Event(n.getAge(),"terminal non-removed")) ;
        }
        else if ( !n.isFossil() && n.isTip() )
        {
            // node is extant leaf : only their number is necessary to compute Lt and Mt
            // events.push_back(Event(n.getAge(),"extant leaf") ;
            // events.push_back(Event(n.getAge(), "terminal non-removed")) ;
            // if (verbose){std::cout << n.getSpeciesName() << std::endl;}
            nb_extant++;
        }
        else if ( n.isInternal() && !n.getChild(0).isSampledAncestor() && !n.getChild(1).isSampledAncestor() )
        {
            // node is a "true" bifurcation event
            events.push_back(Event(n.getAge(),"branching time")) ;
        }
    }

    const RbVector<double> tau = time_points;

    for (size_t i = 0; i < tau.size(); i++)
    {
        events.push_back(Event(tau[i],"time point")) ;
    }

    for ( int i=0; i < occurrence_ages.size(); ++i)
    {
        events.push_back(Event(occurrence_ages[i],"occurrence")) ;
    }

    const std::vector<double> d = timeline;
    for ( int i=1; i < d.size(); i++)
    {
    events.push_back(Event(timeline[i],"rate shift")) ;
    }

    events.push_back(Event(0.0,"present time")) ;

    return (events);
};






/**
 * Compute the joint log-probability density of the observations made up to any time t (direction depending on the algorithm).
 *
 * \param[in]    start_age              Start age of the process.
 * \param[in]    lambda                 Speciation rate.
 * \param[in]    mu                     Extinction rate.
 * \param[in]    psi                    Tree sampling rate.
 * \param[in]    omega                  Occurrence sampling rate.
 * \param[in]    rho                    Sampling probability at present time.
 * \param[in]    removalPr              Removal probability after sampling.
 * \param[in]    maxHiddenLin           Algorithm accuracy (maximal number of hidden lineages).
 * \param[in]    cond                   Condition of the process (none/survival/#Taxa).
 * \param[in]    time_points            Times for which we want to compute the density.
 * \param[in]    useOrigin              If true the start age is the origin time otherwise the root age of the process.
 * \param[in]    useMt                  If true computes densities with the forwards traversal algorithm (Mt) otherwise uses backward one (Lt).
 * \param[in]    verbose                If true displays warnings and information messages.
 * \param[in]    occurrence_ages        Vector of occurrence ages.
 * \param[in]    timeTree               Tree for ancestral populations size inference.
 *
 * \return    The matrix of log-Mt values through time.
*/
MatrixReal RevBayesCore::ComputeLnProbabilityDensitiesOBDP(  const TypedDagNode<double> *start_age,
                                                                      const std::vector<double> &timeline,
                                                                      const std::vector<double> &lambda,
                                                                      const std::vector<double> &mu,
                                                                      const std::vector<double> &psi,
                                                                      const std::vector<double> &omega,
                                                                      const TypedDagNode<double> *rho,
                                                                      const std::vector<double> &removalPr,
                                                                      const TypedDagNode<long> *maxHiddenLin,
                                                                      const std::string &cond,
                                                                      const std::vector<double> &time_points,
                                                                      bool useOrigin,
                                                                      bool useMt,
                                                                      bool verbose,
                                                                      const std::vector<double> &occurrence_ages,
                                                                      const Tree &timeTree)
{
    // Use the forwards traversal algorithm (Mt)
    if (useMt){
        bool returnLogLikelihood = false;    // Input flag

        MatrixReal B_Mt_log = RevBayesCore::ForwardsTraversalMt(start_age, timeline, lambda, mu, psi, omega, rho, removalPr, maxHiddenLin, cond, time_points, useOrigin, returnLogLikelihood, verbose, occurrence_ages, timeTree);

        return (B_Mt_log);
    }
    // Use the backwards traversal algorithm (Lt)
    MatrixReal B_Lt_log = RevBayesCore::BackwardsTraversalLt(start_age, timeline, lambda, mu, psi, omega, rho, removalPr, maxHiddenLin, cond, time_points, useOrigin, verbose, occurrence_ages, timeTree);
    return (B_Lt_log);

};



/**
 * Compute the joint log-probability density of the tree and occurrences.
 *
 * \param[in]    start_age              Start age of the process.
 * \param[in]    lambda                 Speciation rate.
 * \param[in]    mu                     Extinction rate.
 * \param[in]    psi                    Tree sampling rate.
 * \param[in]    omega                  Occurrence sampling rate.
 * \param[in]    rho                    Sampling probability at present time.
 * \param[in]    removalPr              Removal probability after sampling.
 * \param[in]    maxHiddenLin           Algorithm accuracy (maximal number of hidden lineages).
 * \param[in]    cond                   Condition of the process (none/survival/#Taxa).
 * \param[in]    useOrigin              If true the start age is the origin time otherwise the root age of the process.
 * \param[in]    useMt                  If true computes densities with the forwards traversal algorithm (Mt) otherwise uses backward one (Lt).
 * \param[in]    verbose                If true displays warnings and information messages.
 * \param[in]    occurrence_ages        Vector of occurrence ages.
 * \param[in]    timeTree               Tree for ancestral populations size inference.
 *
 * \return    The joint log-likelihood.
*/
double RevBayesCore::ComputeLnLikelihoodOBDP(    const TypedDagNode<double> *start_age,
                                                          const std::vector<double> &timeline,
                                                          const std::vector<double> &lambda,
                                                          const std::vector<double> &mu,
                                                          const std::vector<double> &psi,
                                                          const std::vector<double> &omega,
                                                          const TypedDagNode<double> *rho,
                                                          const std::vector<double> &removalPr,
                                                          const TypedDagNode<long> *maxHiddenLin,
                                                          const std::string &cond,
                                                          bool useOrigin,
                                                          bool useMt,
                                                          bool verbose,
                                                          const std::vector<double> &occurrence_ages,
                                                          const Tree &timeTree)
{
    // Use the forwards traversal algorithm (Mt)
    if (useMt){
        const std::vector<double> time_points_Mt( 1, 0.0 );      // Record the probability density at present to compute the likelihood
        bool returnLogLikelihood = true;                         // Input flag

        MatrixReal LogLikelihood = RevBayesCore::ForwardsTraversalMt(start_age, timeline, lambda, mu, psi, omega, rho, removalPr, maxHiddenLin, cond, time_points_Mt, useOrigin, returnLogLikelihood, verbose, occurrence_ages, timeTree);

        double logLikelihood = LogLikelihood[0][0];
        if (verbose){std::cout << std::setprecision(15) << "\n ==> Log-Likelihood Mt : " << logLikelihood << "\n" << std::endl;}
        return (logLikelihood);
    }
    // Use the backwards traversal algorithm (Lt)
    const std::vector<double> time_points_Lt(1, start_age->getValue());      // Record the probability density at the start age to compute the likelihood

    MatrixReal B_Lt_log = RevBayesCore::BackwardsTraversalLt(start_age, timeline, lambda, mu, psi, omega, rho, removalPr, maxHiddenLin, cond, time_points_Lt, useOrigin, verbose, occurrence_ages, timeTree);

    // The likelihood corresponds to the first element of the B_Lt matrix
    double logLikelihood = B_Lt_log[0][0];
    if (verbose){std::cout << "\n ==> Log-Likelihood Lt : " << logLikelihood << "\n" << std::endl;}
    return (logLikelihood);
};




// /**
//  * Compute the joint probability density of the observations made up to any time t and the population
//  * size at that time, as time decreases towards present, with piecewise constant rates : breadth-first forward traversal algorithm.
//  *
//  * \param[in]    start_age              Start age of the process.
//  * \param[in]    timeline               Rate shifts times.
//  * \param[in]    lambda                 Speciation rate.
//  * \param[in]    mu                     Extinction rate.
//  * \param[in]    psi                    Extinction sampling rate.
//  * \param[in]    omega                  Occurrence sampling rate.
//  * \param[in]    rho                    Sampling probability at present time.
//  * \param[in]    removalPr              Removal probability after sampling.
//  * \param[in]    maxHiddenLin           Algorithm accuracy (maximal number of hidden lineages).
//  * \param[in]    cond                   Condition of the process (none/survival/#Taxa).
//  * \param[in]    time_points            Times for which we want to compute the density.
//  * \param[in]    useOrigin              If true the start age is the origin time otherwise the root age of the process.
//  * \param[in]    timeTree               Tree for ancestral populations size inference.
//  *
//  * \return    The matrix of Mt values through time.
// */
MatrixReal RevBayesCore::ForwardsTraversalMt(   const TypedDagNode<double> *start_age,
                                                                  const std::vector<double> &timeline,
                                                                  const std::vector<double> &lambda,
                                                                  const std::vector<double> &mu,
                                                                  const std::vector<double> &psi,
                                                                  const std::vector<double> &omega,
                                                                  const TypedDagNode<double> *rho,
                                                                  const std::vector<double> &removalPr,
                                                                  const TypedDagNode<long> *maxHiddenLin,
                                                                  const std::string& cond,
                                                                  const std::vector<double> &time_points,
                                                                  bool useOrigin,
                                                                  bool returnLogLikelihood,
                                                                  bool verbose,
                                                                  const std::vector<double> &occurrence_ages,
                                                                  const Tree &timeTree  )
{

    // Construct the vector containig all branching and sampling times + time points for which we want to compute the density.
    std::vector<Event> events = RevBayesCore::PoolEvents(start_age, time_points, occurrence_ages, verbose, timeTree, timeline);

    // Order times oldest to youngest
    struct AgeCompareReverse {
        bool operator()(const Event first, const Event second) {
            return first.time > second.time;
        }
    };

    std::sort( events.begin(), events.end(), AgeCompareReverse() );

    //get rate vectors
    const std::vector<double> birth = lambda;
    const std::vector<double> death = mu;
    const std::vector<double> ps = psi;
    const std::vector<double> om = omega;
    const std::vector<double> rp = removalPr;
    const double rh = rho->getValue();
    const long N = maxHiddenLin->getValue();

    const RbVector<double> tau = time_points;
    const size_t S = tau.size();
    const std::vector<double> gamma = birth + death + ps + om;

    // Initialize an empty matrix and a cursor to write lines in this matrix
    MatrixReal B(S, (N + 1), RbConstants::Double::neginf);
    size_t indxJ = S-1;

    // Count event types :
    size_t k = 1;                       // Number of lineages
    size_t nb_fossil_leafs = 0;         // Number of fossil leafs
    size_t nb_sampled_ancestors = 0;    // Number of sampled ancestors
    size_t nb_occurrences = 0;          // Number of fossil occurrences
    size_t nb_branching_times = 0;      // Number of branching times

    //Initialize rates to their root value and a cursor to update rates
    double birth_current = birth.back();
    double death_current = death.back();
    double ps_current = ps.back();
    double om_current = om.back();
    double rp_current = rp.back();
    double gamma_current = gamma.back();
    size_t indx_rate = timeline.size();

    // Recording the correction terms c to avoid divergence towards extreme values (outside of double precision)
    double log_correction = 0;
    double c;

    double events_factor_log = 0;
    // Estimating the smallest sufficient N value for getting most of the probability density
    size_t N_optimal = 0;
    size_t N_optimal_tmp;

    // We start at the time of origin, supposedly the first time in the vector of events
    RbVector<double> Mt(N+1, 0.0);
    Mt[0] = 1;
    double thPlusOne = events[0].time;
    // if (verbose){std::cout << k << std::endl;}

    if(thPlusOne != start_age->getValue()) {
        if (verbose){std::cout << "WARNING : thPlusOne != start_age : " << thPlusOne << " != " << start_age->getValue() << " - type : " << events[0].type << std::endl;}
    };


    // Then we iterate over the next events
    for(int h = 0; h < events.size(); h++){

      // First, deal with the update on time period (th, thPlusOne)
      double th = events[h].time;
      std::string type = events[h].type;

      // Events older than the start age : accepted
      if(th > start_age->getValue()) {
          if (verbose){std::cout << "WARNING : th > start_age : " << th << " > " << start_age->getValue() << " -> type : " << type << std::endl;}

          // Time points before the start age
          if(type == "time point"){
              for(int i = 0; i < N+1; i++){
                  B[indxJ][i] = log(Mt[i]) + log_correction + events_factor_log;
              }
              indxJ--;
          }
          // Other events before the start age : rejected
          else{
              if(returnLogLikelihood){
                  MatrixReal LogLikelihood_reject(1, 1, RbConstants::Double::neginf);
                  return LogLikelihood_reject;
              }
              else{
                  MatrixReal B_reject(S, (N + 1), RbConstants::Double::neginf);
                  return B_reject;
              }
          }

          continue;
      };

        // Second, deal with the update at punctual event th
        if( th != thPlusOne ){
            MatrixReal A( (N+1), (N+1), 0.0 );

            for(int i = 0; i < (N + 1); i++){
                A[i][i] = gamma_current * (k + i) * (th-thPlusOne);
                if (i < N) A[i][i+1] = -death_current * (i + 1) * (th-thPlusOne);
                if (i > 0) A[i][i-1] = -birth_current * (2*k + i - 1) * (th-thPlusOne);
            }

            RbMath::expMatrixPade(A, A, 4);
            // if (verbose){std::cout << "\nA : " << A << std::endl;}
            // if (verbose){std::cout << "\nMt : " << Mt << std::endl;}
            // if (verbose){std::cout << "\nlog(Mt[0]) : " << log(Mt[0]) << " - log(Mt[1]) : " << log(Mt[1]) << " - log(Mt[2]) : " << log(Mt[2]) << " - log(Mt[3]) : " << log(Mt[3]) << std::endl;}
            Mt = A * Mt;
        }

        if(type == "rate shift"){
        indx_rate--;
        //change rates appropriately, rate changes provided in timeline are in an ascending order
        //size_t where = LocateTimeSliceIndex(th,timeline) - 1;
        birth_current = birth[indx_rate];
        death_current = death[indx_rate];
        ps_current = ps[indx_rate];
        om_current = om[indx_rate];
        rp_current = rp[indx_rate];
        gamma_current = gamma[indx_rate];

        }

        if(type == "time point"){
            // // Combine the scaling factors for all the events until present
            // double events_factor_log = log_correction;
            // // if (verbose){std::cout << "log_correction : " << log_correction << std::endl;}
            // if (ps_current != 0){
            //     events_factor_log += log(ps_current) * (nb_fossil_leafs + nb_sampled_ancestors);
            //     // if (verbose){std::cout << "log(ps) * (nb_fossil_leafs + nb_sampled_ancestors) : " << log(ps) * (nb_fossil_leafs + nb_sampled_ancestors) << std::endl;}
            // }
            // if (om_current != 0){
            //     events_factor_log += log(om_current) * nb_occurrences;
            //     // if (verbose){std::cout << "log(om) * nb_occurrences : " << log(om) * nb_occurrences << std::endl;}
            // }
            // if (birth_current != 0){
            //     events_factor_log += log(birth_current) * nb_branching_times;
            //     // if (verbose){std::cout << "log(birth) * nb_branching_times : " << log(birth) * nb_branching_times << std::endl;}
            // }
            // // if (verbose){std::cout << "events_factor_log : " << events_factor_log << std::endl;}
            for(int i = 0; i < N+1; i++){
                B[indxJ][i] = log(Mt[i]) + events_factor_log + log_correction;
            }
            // if (verbose){std::cout << "Time point scaled log : log(Mt[0]) + events_factor_log : " << log(Mt[0]) + events_factor_log << " / log(Mt[N]) + events_factor_log : " << log(Mt[N]) + events_factor_log << std::endl;}
            indxJ--;
        }

        if(type == "fossil leaf"){
            for(int i = N; i > 0 ; i--){
                // Mt[i] = Mt[i-1] * ps * (1-rp) + ps * rp * Mt[i];
                Mt[i] = Mt[i-1] * (1-rp_current) + rp_current * Mt[i];
            }
            // Mt[0] *= ps * rp;
            Mt[0] *= rp_current;
            k--;
            //nb_fossil_leafs++;
            if(ps_current!=0){events_factor_log+=log(ps_current);}
        }

        if(type == "sampled ancestor"){
            for(int i = 0; i < N+1; i++){
                // Mt[i] *= ps * (1-rp);
                Mt[i] *= (1-rp_current);
            }
            //nb_sampled_ancestors++;
            if(ps_current!=0){events_factor_log+=log(ps_current);}
        }

        if(type == "occurrence"){
            for(int i = 0; i < N; i++){
                // Mt[i] = Mt[i+1] * (i+1) * om * rp + (k+i) * om * (1-rp) * Mt[i];
                Mt[i] = Mt[i+1] * (i+1) * rp_current + (k+i) * (1-rp_current) * Mt[i];
            }
            // Mt[N] *= (k+N) * om * (1-rp);
            Mt[N] *= (k+N) * (1-rp_current);
            //nb_occurrences++;
            if(om_current!=0){events_factor_log+=log(om_current);}
         }

        if(type == "branching time"){
            // for(int i = 0; i < N+1; i++){
            //     Mt[i] *= birth;
            // }
            k++;
            //nb_branching_times++;
            if(birth_current!=0){events_factor_log+=log(birth_current);}
        }

        // Correct probability vector to avoid divergence towards extreme values (outside of double precision) by scaling to a unit maximum value
        double max_Mt = *std::max_element(Mt.begin(), Mt.end());
        c = 1/max_Mt;
        for(int i = 0; i < N+1; i++){
            Mt[i] = Mt[i] * c;
        }
        log_correction -= log(c);

        // if (verbose){std::cout << "Event time : " << th << " - Event type : " << type << " -> log(c) : " << log(c) << " -> Mt[0] : " << Mt[0] << " / Mt[1] : " << Mt[1] << " / Mt[N] : " << Mt[N] << std::endl;}

        // Check that N is big enough
        if (Mt[N]>0.001){
            // In that case the optimal N value cannot be estimated because it is greater than the chosen one
            N_optimal = N+1;
        }
        else{
            N_optimal_tmp = N;
            while (Mt[N_optimal_tmp-1]<0.001){
                N_optimal_tmp--;
            }
            N_optimal = std::max(N_optimal, N_optimal_tmp);
             // if (verbose){std::cout << "\nThe smallest sufficient N value ( such as Mt[N_optimal] < max(Mt)/1000 for all t ) is " << N_optimal << "\n" << std::endl;}
        }

        thPlusOne = th;
    }

    // Give the estimated optimal N value
    if ((N_optimal < N) & verbose){
      std::cout << "\nTo improve performance, set N to a lower value -> current value : N = " << N << ", optimal value ( such as Mt[N_optimal] < max(Mt)/1000 for all t ) : N_optimal = " << N_optimal << "\n" << std::endl;
    }
    else if ((N_optimal == N) & verbose){
      std::cout << "\nThe chosen N value is optimal ( such as Mt[N_optimal] < max(Mt)/1000 for all t ) : N = N_optimal = " << N_optimal << std::endl;
    }
    else if (N_optimal == N+1){
      std::cout << "\nWARNING : There is a time t at which Mt[N] contains a non-negligeable probability ( Mt[N] > max(Mt)/1000 ) -> you should increase N\n" << std::endl;
    }
///////////////////// + log factor
    if(returnLogLikelihood){
        // double events_factor_log = log_correction;
        //
        // if (ps_current != 0){events_factor_log += log(ps_current) * (nb_fossil_leafs + nb_sampled_ancestors); }
        // if (om_current != 0){events_factor_log += log(om_current) * nb_occurrences; }
        // if (birth_current != 0) {events_factor_log += log(birth_current) * nb_branching_times; }

        double likelihood = 0;
        for(int i = 0; i < N+1; i++){
            likelihood += Mt[i] * pow(rh, k) * pow(1.0 - rh, i);
        }
        std::cout<<"events_factor_log Mt="<<events_factor_log<<std::endl;
        std::cout<<"log corr="<<log_correction<<std::endl;
        std::cout<<"Sum"<<log_correction+events_factor_log<<std::endl;


        MatrixReal LogLikelihood(1, 1, log(likelihood) + events_factor_log + log_correction );
        return LogLikelihood;
    }

    return B;
}

//
// /**
//  * Compute the probability density of observations made down any time t, conditioned on the population
//  * size at that time, as time increases towards the past, with piecewise constant rates : breadth-first backward traversal algorithm.
//  *
//  * \param[in]    start_age              Start age of the process.
//  * \param[in]    timeline               Rate shifts times.
//  * \param[in]    lambda                 Speciation rate.
//  * \param[in]    mu                     Extinction rate.
//  * \param[in]    psi                    Extinction sampling rate.
//  * \param[in]    omega                  Occurrence sampling rate.
//  * \param[in]    rho                    Sampling probability at present time.
//  * \param[in]    removalPr              Removal probability after sampling.
//  * \param[in]    maxHiddenLin           Algorithm accuracy (maximal number of hidden lineages).
//  * \param[in]    cond                   Condition of the process (none/survival/#Taxa).
//  * \param[in]    time_points            Times for which we want to compute the density.
//  * \param[in]    useOrigin              If true the start age is the origin time otherwise the root age of the process.
//  * \param[in]    timeTree               Tree for ancestral populations size inference.
//  *
//  * \return    The matrix of Lt values through time.
//  */
MatrixReal RevBayesCore::BackwardsTraversalLt(  const TypedDagNode<double> *start_age,
                                                                  const std::vector<double> &timeline,
                                                                  const std::vector<double> &lambda,
                                                                  const std::vector<double> &mu,
                                                                  const std::vector<double> &psi,
                                                                  const std::vector<double> &omega,
                                                                  const TypedDagNode<double> *rho,
                                                                  const std::vector<double> &removalPr,
                                                                  const TypedDagNode<long> *maxHiddenLin,
                                                                  const std::string& cond,
                                                                  const std::vector<double> &time_points,
                                                                  bool useOrigin,
                                                                  bool verbose,
                                                                  const std::vector<double> &occurrence_ages,
                                                                  const Tree &timeTree  )
{

    // Construct the vector containig all branching and sampling times + time points for which we want to compute the density.
    std::vector<Event> events = RevBayesCore::PoolEvents(start_age, time_points, occurrence_ages, verbose, timeTree, timeline);

    // Order times youngest to oldest
    struct AgeCompare {
        bool operator()(const Event first, const Event second) {
            return first.time < second.time;
        }
    };
    std::sort( events.begin(), events.end(), AgeCompare() );

    //get rate vectors
    const std::vector<double> birth = lambda;
    const std::vector<double> death = mu;
    const std::vector<double> ps = psi;
    const std::vector<double> om = omega;
    const std::vector<double> rp = removalPr;
    const double rh = rho->getValue();

    const RbVector<double> tau = time_points;
    const long N = maxHiddenLin->getValue();
    const size_t S = tau.size();

    const std::vector<double> gamma = birth + death + ps + om;

    // Count the number of extant lineages
    size_t k = timeTree.getNumberOfExtantTips();

    // Initialize an empty matrix and a cursor indxJ to write lines in this matrix
    MatrixReal B(S, (N + 1), RbConstants::Double::neginf);
    size_t indxJ = 0;


    // Initialize rates to their present value and a cursor for rate updates
    double birth_current = birth[0];
    double death_current = death[0];
    double ps_current = ps[0];
    double om_current = om[0];
    double rp_current = rp[0];
    double gamma_current = gamma[0];
    size_t indx_rate=0;


    // Recording the correction terms c to avoid divergence towards extreme values (outside of double precision)
    double log_correction = 0;
    double c;

    // Estimating the smallest sufficient N value for getting most of the probability density
    size_t N_optimal = 0;
    size_t N_optimal_tmp;

    // We start at time 0 with type "present" in the vector of events
    double thMinusOne = events[0].time;
    RbVector<double> Lt(N+1, pow(rh, k));
    for ( int i = 0; i < (N + 1); i++){
        Lt[i] *= pow( 1.0-rh, i );
    }
    // std::cout << "Event time : " << events[0].time << " - Event type : " << events[0].type << std::endl;

    // Recording the log terms introduced by events
    double events_factor_log = 0;

    // We then iterate over the following events until finding the time of origin
    for(int h = 0; h < events.size(); h++){

      double th = events[h].time;
      std::string type = events[h].type;

      // Events older than the start age
      if(th > start_age->getValue()) {
          if (verbose){std::cout << "WARNING : th > start_age : " << th << " > " << start_age->getValue() << " -> type : " << type << std::endl;}

          // Time points older than the start age : accepted
          if(type == "time point"){

              for(int i = 0; i < N+1; i++){
                  B[indxJ][i] = log(Lt[i]) + events_factor_log + log_correction;
              }
              indxJ++;
          }
          // Other events older than the start age : rejected
          else{
              MatrixReal B_reject(S, (N + 1), RbConstants::Double::neginf);
              return B_reject;
          }

          continue;
      };

        // First deal with the update along (thMinusOne, th)
        if( th != thMinusOne ){

            MatrixReal A( (N+1), (N+1), 0.0 );
            for(int i = 0; i < (N + 1); i++){
              A[i][i] = -gamma_current * (k + i) * (th - thMinusOne);
              if (i < N) A[i][i+1] = birth_current * ( (2 * k) + i ) * (th - thMinusOne);
              if (i > 0) A[i][i-1] = death_current * i * (th - thMinusOne);

            }
            RbMath::expMatrixPade(A, A, 4);
            Lt = A * Lt;

        }

        // Second, deal with the update at time th
        if(type == "time point"){
            // if (verbose){std::cout << "events_factor_log : " << events_factor_log  << " / Lt : " << Lt << std::endl;}
            for(int i = 0; i < N+1; i++){
                B[indxJ][i] = log(Lt[i]) + events_factor_log + log_correction;
            }
            // if (verbose){std::cout << "Time point scaled log : log(Lt[0]) + events_factor_log : " << log(Lt[0]) + events_factor_log << " / log(Lt[N]) + events_factor_log : " << log(Lt[N]) + events_factor_log << std::endl;}
            indxJ++;
        }

        if(type == "rate shift"){
        indx_rate++;
        //change rates appropriately
        birth_current = birth[indx_rate];
        death_current = death[indx_rate];
        ps_current = ps[indx_rate];
        om_current = om[indx_rate];
        rp_current = rp[indx_rate];
        gamma_current = gamma[indx_rate];

        }

       if(type == "fossil leaf"){
           for(int i = 0; i < N; i++){
               // Lt[i] = Lt[i] * ps * rp + Lt[i+1] * ps * (1.0-rp) ;
               Lt[i] = Lt[i] * rp_current + Lt[i+1] * (1.0-rp_current) ;
           }
           // Lt[N] = Lt[N] * ps * rp;
           Lt[N] = Lt[N] * rp_current;
           k++;
           if (ps_current!=0){events_factor_log += log(ps_current);};
       }

       if(type == "sampled ancestor"){
           for(int i = 0; i < N+1; i++){
               // Lt[i] = Lt[i] * ps * (1.0-rp);
               Lt[i] = Lt[i] * (1.0-rp_current);
           }
           if (ps_current!=0){events_factor_log += log(ps_current);};
       }

       if(type == "occurrence"){
           for(int i = N; i > 0; i--){
               // Lt[i] = Lt[i-1] * i * om * rp + Lt[i] * (k+i) * om * (1-rp);
               Lt[i] = Lt[i-1] * i * rp_current + Lt[i] * (k+i) * (1-rp_current);
           }
           // Lt[0] = Lt[0] * k * om * (1-rp);
           Lt[0] = Lt[0] * k * (1-rp_current);
           if (om_current!=0){events_factor_log += log(om_current);};
        }

        if(type == "branching time"){
            // for(int i = 0; i < N+1; i++){
            //     Lt[i] = Lt[i] * birth;
            // }
            Lt *= birth_current;
            k--;
          if (birth_current!=0){events_factor_log += log(birth_current);};
        }

        // Correct probability vector to avoid divergence towards extreme values (outside of double precision) by scaling to a unit maximum value
        double max_Lt = *std::max_element(Lt.begin(), Lt.end());
        c = 1/max_Lt;
        for(int i = 0; i < N+1; i++){
            Lt[i] = Lt[i] * c;
        }
        log_correction -= log(c);

        // if (verbose){std::cout << "Event time : " << th << " - Event type : " << type << " -> log(c) : " << log(c) << " -> Lt[0] : " << Lt[0] << " / Lt[1] : " << Lt[1] << " / Lt[N-1] : " << Lt[N-1] << " / Lt[N] : " << Lt[N] << std::endl;}

        // Check that N is big enough
        if (Lt[N]>0.001){
            // In that case the optimal N value cannot be estimated because it is greater than the chosen one
            N_optimal = N+1;
        }
        else{
            N_optimal_tmp = N;
            while (Lt[N_optimal_tmp-1]<0.001){
                N_optimal_tmp--;
            }
            N_optimal = std::max(N_optimal, N_optimal_tmp);
            // if (verbose){std::cout << "\nThe smallest sufficient N value ( such as Lt[N_optimal] < max(Lt)/1000 for all t ) is " << N_optimal << "\n" << std::endl;}
        }

        thMinusOne = th;
    }
    std::cout<<"events_factor_log LT="<<events_factor_log<<std::endl;
    std::cout<<"log_correction LT="<<log_correction<<std::endl;

    // Give the estimated optimal N value
    if ((N_optimal < N) & verbose){
      std::cout << "\nTo improve performance, set N to a lower value -> current value : N = " << N << ", optimal value ( such as Lt[N_optimal] < max(Lt)/1000 for all t ) : N_optimal = " << N_optimal << "\n" << std::endl;
    }
    else if ((N_optimal == N) & verbose){
      std::cout << "\nThe chosen N value is optimal ( such as Lt[N_optimal] < max(Lt)/1000 for all t ) : N = N_optimal = " << N_optimal << std::endl;
    }
    else if (N_optimal == N+1){
      std::cout << "\nWARNING : There is a time t at which Lt[N] contains a non-negligeable probability ( Lt[N] > max(Lt)/1000 ) -> you should increase N\n" << std::endl;
    }

    return B;
}


/**
 * Compute the joint log probability density of observations made down any time t, conditioned on the population
 * size at that time, as time increases towards the past : breadth-first backwards traversal algorithm.
 * This function is borrowed from Ankit Gupta,
 * see his paper: "The probability distribution of the reconstructed phylogenetic tree with occurrence data".
 * \param[in]    start_age              Start age of the process.
 * \param[in]    timeline               Rate shifts times.
 * \param[in]    lambda                 Speciation rate.
 * \param[in]    mu                     Extinction rate.
 * \param[in]    psi                    Extinction sampling rate.
 * \param[in]    omega                  Occurrence sampling rate.
 * \param[in]    rho                    Sampling probability at present time.
 * \param[in]    removalPr              Removal probability after sampling.
 * \param[in]    maxHiddenLin           Algorithm accuracy (maximal number of hidden lineages).
 * \param[in]    cond                   Condition of the process (none/survival/#Taxa).
 * \param[in]    time_points            Times for which we want to compute the density.
 * \param[in]    useOrigin              If true the start age is the origin time otherwise the root age of the process.
 * \param[in]    timeTree               Tree for ancestral populations size inference.
 *
 * \return    The matrix of Lt values through time.
 */
double RevBayesCore::likelihoodWithAllSamplesRemoved(  const TypedDagNode<double> *start_age,
                                                                  const std::vector<double> &timeline,
                                                                  const std::vector<double> &lambda,
                                                                  const std::vector<double> &mu,
                                                                  const std::vector<double> &psi,
                                                                  const std::vector<double> &omega,
                                                                  const TypedDagNode<double> *rho,
                                                                  const std::vector<double> &removalPr,
                                                                  const std::string& cond,
                                                                  const std::vector<double> &time_points,
                                                                  bool useOrigin,
                                                                  bool verbose,
                                                                  const std::vector<double> &occurrence_ages,
                                                                  const Tree &timeTree  )
{

    // Construct the vector containig all branching and sampling times + time points for which we want to compute the density.
    std::vector<Event> events = RevBayesCore::PoolEvents(start_age, time_points, occurrence_ages, verbose, timeTree, timeline);

    // Order times youngest to oldest
    struct AgeCompare {
        bool operator()(const Event first, const Event second) {
            return first.time < second.time;
        }
    };
    std::sort( events.begin(), events.end(), AgeCompare() );

    // this variable is used to check that r==1 throughout and that the data can be observed with r=1.
    bool impossible = false;

    //get rate vectors
    const std::vector<double> birth = lambda;
    const std::vector<double> death = mu;
    const std::vector<double> ps = psi;
    const std::vector<double> om = omega;
    const std::vector<double> rp = removalPr;
    const double rh = rho->getValue();
    // Initialize rates to their present value
    double birth_current = birth[0];
    double death_current = death[0];
    double ps_current = ps[0];
    double om_current = om[0];
    double rp_current = rp[0];
    if(rp_current != 1.0){
        std::cout << "Warning ! This function should be used only when r=1. Here, r = " << rp_current << std::endl;
        impossible = true;
    }
    size_t indx_rate=0;

    // Count the number of extant lineages
    size_t k = timeTree.getNumberOfExtantTips();

    // starts to compute the likelihood
    double LogLikelihood = k*log(rh);
    double log_factor = 0.0;
    std::vector<double> v(1, 1);

    double p0 = 1-rh;
    double oldp0 = p0;

    // We start at time 0 with type "present" in the vector of events
    double thMinusOne = events[0].time;
    double deltat = 0;

    // We then iterate over the following events until finding the time of origin
    for(int h = 0; h < events.size(); h++){

        double th = events[h].time;
        std::string type = events[h].type;

        // Events older than the start age
        if(th > start_age->getValue()) {
            if (verbose){std::cout << "WARNING : th > start_age : " << th << " > " << start_age->getValue() << " -> type : " << type << std::endl;}
            continue;
        };

        if( th != thMinusOne ){
            oldp0 = p0;
            deltat = th-thMinusOne;
            p0 = GetP0(deltat, birth_current, oldp0, death_current, ps_current+om_current, 0);
            v = TransformDerivativeContrVec(deltat, birth_current, oldp0, death_current, ps_current+om_current, 0, k, v);
        }

        if(type == "rate shift"){
            indx_rate++;
            birth_current = birth[indx_rate];
            death_current = death[indx_rate];
            ps_current = ps[indx_rate];
            om_current = om[indx_rate];
            rp_current = rp[indx_rate];
            if(rp_current != 1.0){
                std::cout << "Warning ! This function should be used only when r=1. Here, r = " << rp_current << std::endl;
                impossible = true;
            }
        }


        if(type == "fossil leaf"){
           k++;
           log_factor += log(ps_current);
       }

        if(type == "sampled ancestor"){
           std::cout << "Warning ! It should not be possible to see a sampled ancestor here. This function only deals with r = 1" << std::endl;
           log_factor += log(ps_current);
           impossible = true;
        }

        if(type == "occurrence"){
           std::cout << v.size() << std::endl;
           v.insert(v.begin(), 0.0);
           log_factor += log(om_current);
        }

        if(type == "branching time"){
           k--;
           log_factor += log(birth_current);
        }

        thMinusOne = th;
    }

    if (impossible){
      LogLikelihood = -1e500;
    }else{
      LogLikelihood += log(v[0]) + log_factor;
    }

    return LogLikelihood;
}


// Below are the helper functions we need for analytical computation
double RevBayesCore::GetQ(  const double t, const double beta, const double rhoc, const double mu, const double psi, const double omega )
{
    double c1 = std::sqrt( (beta - mu - omega - psi)*(beta - mu - omega - psi)+4*beta*psi);
    double c2 = (beta + mu + omega + psi - 2*beta*rhoc)/c1;
    double q = 2*(1-c2*c2) + exp(-c1*t)*(1-c2)*(1-c2) + exp(c1*t)*(1+c2)*(1+c2);
    return q;
}

double RevBayesCore::GetDerivativeQ(  const double t, const double beta, const double rhoc, const double mu, const double psi, const double omega, const unsigned n )
{
    double c1 = std::sqrt( (beta - mu - omega - psi)*(beta - mu - omega - psi)+4*beta*psi);
    double c2 = (beta + mu + omega + psi - 2*beta*rhoc)/c1;
    double q = 0.0;
    if(n == 1){
        q = -(4*beta/c1)*( exp(c1*t) -exp(-c1*t) + c2*( exp(c1*t) + exp(-c1*t) - 2 )    );
    }else if(n == 2){
        q = 8*( (beta/c1)*(beta/c1) )*(exp(c1*t) + exp(-c1*t) - 2);
    }
    return q;
}

unsigned RevBayesCore::nChoosek( unsigned n, unsigned k )
{
    if (k > n) return 0;
    if (k * 2 > n) k = n-k;
    if (k == 0) return 1;
    unsigned result = n;
    for( unsigned i = 2; i <= k; ++i ) {
        result *= (n-i+1);
        result /= i;
    }
    return result;
}

unsigned RevBayesCore::factorial( unsigned n )
{
    unsigned long long res = 1;
    for(int i = 1; i<=n ; i++){
        res *= i;
    }
    return res;
}

double RevBayesCore::GetMultiDerivativeRecQ(  const double t, const double beta, const double rhoc, const double mu, const double psi, const double omega, const unsigned n, const unsigned NumObservedLineages )
{
    double q0 = 1. / GetQ(t,beta,rhoc,mu,psi,omega);
    double q = 0.0;
    if (n == 0){
        q = std::pow(q0, NumObservedLineages);
    }
    else if (NumObservedLineages == 0){
        q = 0;
    }
    else{
        double Qder1 = GetDerivativeQ(t,beta,rhoc,mu,psi,omega,1);
        double Qder2 = GetDerivativeQ(t,beta,rhoc,mu,psi,omega,2);
        for( unsigned m1 =0; m1 <= n; m1++){
            if( (n-m1) % 2 == 0){
                unsigned m2 = (n-m1)/2;
                q += std::pow(-1, m1+m2) / std::pow(2,m2) * std::pow(q0, m1 + m2 + NumObservedLineages) * nChoosek(m1+m2+NumObservedLineages-1,m1+m2)*nChoosek(m1+m2,m1) *  std::pow(Qder1, m1) * std::pow(Qder2, m2);
            }
        }
    }
    q *= std::pow( GetQ(0,beta,rhoc,mu,psi,omega), NumObservedLineages ) * factorial(n);
    return q;
}


double RevBayesCore::GetP0(  const double t, const double beta, const double rhoc, const double mu, const double psi, const double omega)
{
    double c1 = std::sqrt( (beta - mu - omega - psi)*(beta - mu - omega - psi)+4*beta*psi);
    double c2 = (beta + mu + omega + psi - 2*beta*rhoc)/c1;
    double factor = ( exp(c1*t)*(1+c2)*(1+c2) - exp(-c1*t)*(1-c2)*(1-c2)) / GetQ(t,beta,rhoc,mu,psi,omega);
    double p0 = (1/(2*beta))*( beta + mu +omega + psi)  - (c1/(2*beta))*factor;
    return p0;
}


double RevBayesCore::GetDerP0(  const double t, const double beta, const double rhoc, const double mu, const double psi, const double omega, const unsigned n)
{
    double c1 = std::sqrt( (beta - mu - omega - psi)*(beta - mu - omega - psi)+4*beta*psi);
    double c2 = (beta + mu + omega + psi - 2*beta*rhoc)/c1;

    double derfac = -2*beta/c1;
    std::vector<double> factor(n+1, 0.0);
    factor[0] = ( exp(c1*t)*(1+c2)*(1+c2) - exp(-c1*t)*(1-c2)*(1-c2) ) / 4;
    factor[1] =  derfac*( exp(c1*t)*(1+c2) + exp(-c1*t)*(1-c2))/2;
    factor[2] = (derfac * derfac)*( exp(c1*t) - exp(-c1*t))/2;
    double derp0 = 0.0;
    for (unsigned i = 0; i<= n; i++){
        derp0 += nChoosek(n,i)*factor[i]*GetMultiDerivativeRecQ(t,beta,rhoc,mu,psi,omega,n-i,1);
    }
    derp0 = -derp0*(c1/(2*beta));
    return derp0;
}


std::vector<double> RevBayesCore::TransformDerivativeContrVec( const double t, const double beta, const double rhoc, const double mu, const double psi, const double omega, const unsigned NumObservedLineages, const std::vector<double> v )
{
    unsigned n = v.size();
    std::vector<double> v1(n, 0.0);

    if(n > 1){
        // requires computation of (n-1) derivatives
        std::vector<double> derivativeVector(n-1, 0.0);
        for (unsigned i=0; i< n-1; i++){
            derivativeVector[i] = GetDerP0(t,beta,rhoc,mu,psi,omega,i+1);
        }
        //std::cout << derivativeVector << std::endl;
        MatrixReal B = IncompleteBellPolynomial(n-1,n-1,derivativeVector);
        //std::cout << B << std::endl;
        MatrixReal A(n, n, 0.0);

        for (unsigned i = 0; i < n; i++){
            for (unsigned j = 0; j <= i; j++){
                A[i][j] = nChoosek(i,j) * GetMultiDerivativeRecQ(t,beta,rhoc,mu,psi,omega,i-j,NumObservedLineages);
            }
        }

        MatrixReal C = A*B;
        //v1 = (C')*v;
        for( int i = 0; i < n; i++){
            for (int k = 0; k < n; k++){
                v1[i] += C[k][i] * v[k];
            }
        }
    }
    else{
        v1 = GetMultiDerivativeRecQ(t,beta,rhoc,mu,psi,omega,0,NumObservedLineages)*v;
    }
    return v1;
}

MatrixReal RevBayesCore::IncompleteBellPolynomial(unsigned N, unsigned K, std::vector<double> Vector)
{
    MatrixReal A(N, K, 0.0);
    for (int i = 0; i < N; i++){
        A[i][0] = Vector[i];
    }

    for (int n = 1; n < N; n++){
        for (int k = 1; k < K; k++){
            for (int m = 1; m <= n-k+1; m++){
                A[n][k] += nChoosek(n, m-1) * Vector[m-1] * A[n-m][k-1];
            }
        }
    }

    MatrixReal OutputMatrix(N+1, K+1, 0.0);
    OutputMatrix[0][0] = 1.;
    for (int i = 1; i<N+1; i++){
        for (int j = 1; j<K+1; j++){
            OutputMatrix[i][j] = A[i-1][j-1];
        }
    }

    return OutputMatrix;
}