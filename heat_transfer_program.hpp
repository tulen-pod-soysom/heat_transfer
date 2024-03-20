#include <list>
#include <vector>

#include <math_functions.hpp>
#include <time_flow_program.hpp>
#include <physics/boundary_condition.hpp>
#include <physics/tridiagonal_matrix_algorithm.hpp>
#include <physics/dimensionless.hpp>
#include <linspace.hpp>

#include <boost/numeric/ublas/matrix.hpp>

struct rect
{
    double left,top,right,bottom;
};

struct parameters : public dimensionless_variables
{
    double height{1};
    double radius{1};

    double wall_width{0.1};

    double heater_height{0.4};
    double heater_radius{0.2};

    double heater_power{1};

    struct material
    {
         double thermal_conductivity;
         double thermal_capacity;
        
         double lambda2;

        material(double conductivity,double capacity) : 
        thermal_conductivity(conductivity),
        thermal_capacity(capacity),
        lambda2((thermal_conductivity)/(thermal_capacity)){}

        material(const material& m) : material(m.thermal_conductivity,m.thermal_capacity){}
    };
    
    material glass{ 1.15, 840};
    material metal{15.00, 500};
    material liquid{ 0.60,4200};

    double external_temperature{1};

    double epsilon{0.001};

    unsigned r_divisions{64};
    unsigned z_divisions{64};
    double t_step{8e-6};

    def_variable(t,t0,1); //???
    def_variable(z,z0,sqrt(liquid.thermal_conductivity/liquid.thermal_capacity * t0));
    def_variable(T,T0,1); //basically does nothing...
};

typedef boost::numeric::ublas::matrix<double> mat;

struct variables
{
    std::list<mat> temperature_field;
    
    discrete_linspace r,z;
    double t = {};

    rect heater_rect;
    rect steel_rect;
};

class heat_transfer_program : public time_flow_program<parameters,variables>
{
public:
    void init()
    {
        v.r.create_bound_dependent(0,p.radius,p.r_divisions,true);
        v.z.create_bound_dependent(0,p.height,p.z_divisions,true);
        v.temperature_field.clear();

        v.temperature_field.emplace_back(v.r.size(),v.z.size(),p.external_temperature);
        v.t = 0;

        v.heater_rect = {0,p.height - p.wall_width,p.heater_radius,p.height - p.wall_width - p.heater_height};
        v.steel_rect = {0,p.wall_width,p.radius - p.wall_width,p.height-p.wall_width};
    }

    parameters::material& material_at_point(double r, double z)
    {
        if (r > p.radius - p.wall_width) return p.metal;
        if ((z < p.wall_width) || (z > p.height - p.wall_width)) return p.metal;
        if ((r < p.heater_radius) && (z > p.height - p.wall_width - p.heater_height) && (z < p.height - p.wall_width)) return p.metal;
        else return p.liquid;
    }

    double heat_power_func(double r, double z)
    {
        if (r < p.heater_radius)
            if ((z > p.height - p.wall_width - p.heater_height) && (z < p.height - p.wall_width))
                return p.heater_power * p.t0 / p.metal.thermal_capacity / p.T0;
        return 0.0;
    }

    double lambda2_func(double r, double z)
    {
        if (r > p.radius - p.wall_width) return p.metal.lambda2;
        if ((z < p.wall_width) && (z > p.height - p.wall_width)) return p.metal.lambda2;
        if ((r < p.heater_radius) && (z > p.height - p.wall_width - p.heater_height) && (z < p.height - p.wall_width)) return p.metal.lambda2;
        else return p.liquid.lambda2;
    }

    void cycle_function()
    {
        auto& prev_T = *v.temperature_field.rbegin();

        mat T(v.r.size(),v.z.size());

        mat dTdr    (v.r.size(),v.z.size());
        mat dTdz    (v.r.size(),v.z.size());
        mat d2Tdr2  (v.r.size(),v.z.size());
        mat d2Tdz2  (v.r.size(),v.z.size());

        for (size_t i = 0; i < v.z.size(); i++) my_functions::differentiate(v.r.get_step(),(prev_T.begin2()+i).begin(),(prev_T.begin2()+i).end(),(dTdr.begin2()+i).begin());
        for (size_t i = 0; i < v.r.size(); i++) my_functions::differentiate(v.z.get_step(),(prev_T.begin1()+i).begin(),(prev_T.begin1()+i).end(),(dTdz.begin1()+i).begin());
        for (size_t i = 0; i < v.z.size(); i++) my_functions::differentiate(v.r.get_step(),(dTdr.begin2()+i).begin(),(dTdr.begin2()+i).end(),(d2Tdr2.begin2()+i).begin());
        for (size_t i = 0; i < v.r.size(); i++) my_functions::differentiate(v.z.get_step(),(dTdz.begin1()+i).begin(),(dTdz.begin1()+i).end(),(d2Tdz2.begin1()+i).begin());


        auto l2 = [&](unsigned i, unsigned j) {return material_at_point(v.r[i],v.z[j]).lambda2 /p.liquid.lambda2;};
        auto Q = [&](unsigned i, unsigned j){return heat_power_func(v.r[i],v.z[j])/material_at_point(v.r[i],v.z[j]).thermal_capacity;};
        double& dt = p.t_step;
        double dr = v.r.get_step();
        double dz = v.z.get_step();


        auto& e = p.epsilon;
        auto& t_e = p.external_temperature;
        auto& k_m = p.metal.thermal_conductivity;
        auto& k_g = p.glass.thermal_conductivity;

        boundary_condition_first_order left_border(1.0,0.0);
        boundary_condition_first_order bottom_border(1.0,0.0);

        boundary_condition_first_order right_border(//1,0
            1.0/(1+e*dr/k_m),
            t_e/(k_m/e/dr + 1)
        );

        boundary_condition_first_order upper_border(//1,0
            1.0/(1+e*dr/k_m),
            t_e/(k_m/e/dr + 1)
        );

        // time step [t_i -> t_i+0.5*dt]
        run_through_method<double> by_r;
        for (size_t j = 0; j < v.z.size(); j++)
        {
            //double Q = heat_power_func(); //Функция мощности тепла
            by_r.A = [&](unsigned i){ return    -l2(i,j) * dt / 4.0 * (1.0 / dr / dr - 0.5 / dr / v.r[i]); };
            by_r.B = [&](unsigned i){ return 1 + l2(i,j) * dt / 2.0 / dr / dr; };
            by_r.C = [&](unsigned i){ return    -l2(i,j) * dt / 4.0 * (1.0 / dr / dr + 0.5 / dr / v.r[i]); };
            by_r.D = [&](unsigned i){ return    prev_T(i, j) + dt / 4.0 * (l2(i,j)*(d2Tdr2(i,j) + dTdr(i,j)/v.r[i] + 2*d2Tdz2(i,j))+ 2*Q(i,j)); };



            by_r.evaluate(v.r.size(),
                          1./left_border.mu,left_border.nu/left_border.mu,
                          right_border.mu,right_border.nu
                          );
            std::move(by_r.output.begin(),by_r.output.end(),(T.begin2()+j).begin());
        }

        for (size_t i = 0; i < v.z.size(); i++) my_functions::differentiate(v.r.get_step(),(T.begin2()+i).begin(),(T.begin2()+i).end(),(dTdr.begin2()+i).begin());
        for (size_t i = 0; i < v.r.size(); i++) my_functions::differentiate(v.z.get_step(),(T.begin1()+i).begin(),(T.begin1()+i).end(),(dTdz.begin1()+i).begin());
        for (size_t i = 0; i < v.z.size(); i++) my_functions::differentiate(v.r.get_step(),(dTdr.begin2()+i).begin(),(dTdr.begin2()+i).end(),(d2Tdr2.begin2()+i).begin());
        for (size_t i = 0; i < v.r.size(); i++) my_functions::differentiate(v.z.get_step(),(dTdz.begin1()+i).begin(),(dTdz.begin1()+i).end(),(d2Tdz2.begin1()+i).begin());



        // time step [t_i+0.5*dt -> t_i+1]
        run_through_method<double> by_z;    
        for (size_t i = 1; i < v.z.size(); i++)
        {
            by_z.A = [&](unsigned j){return    -l2(i,j)*dt/4.0/dz/dz;};
            by_z.B = [&](unsigned j){return 1 + l2(i,j)*dt/2.0/dz/dz;};
            by_z.C = [&](unsigned j){return    -l2(i,j)*dt/4.0/dz/dz;};
            by_z.D = [&](unsigned j){return T(i,j) + dt/2.0 * (l2(i,j)*(d2Tdr2(i,j)+dTdr(i,j)/v.r[i] + d2Tdz2(i,j)/2.0)+Q(i,j));};

            

            by_z.evaluate(v.z.size(),
                          1.0/bottom_border.mu,bottom_border.nu/bottom_border.mu,
                          upper_border.mu,upper_border.nu
                          );
            std::move(by_z.output.begin(),by_z.output.end(),(T.begin1()+i).begin());
        }


        //for (int i = 0; i < v.r.size(); ++i) {
        //    T(i,0) = left_and_bottom_border(T(i,1));
        //}
        //
        //for (int j = 0; j < v.z.size(); ++j) {
        //    T(0,j) = left_and_bottom_border(T(1,j));
        //}



        //for (int j = 0; j < T.size2(); ++j) {
        //    T(T.size1()-1,j) = right_border(T(T.size1()-2,j));
        //}
        //
        //for (int i = 0; i < T.size1(); ++i) {
        //    T(i,T.size2()-1) = upper_border(T(i,T.size2()-2));
        //}


        boundary_condition_second_order steel_to_water(p.metal.thermal_conductivity,p.liquid.thermal_conductivity);
        boundary_condition_second_order steel_to_glass(p.metal.thermal_conductivity,p.glass.thermal_conductivity);
        boundary_condition_second_order water_to_glass(p.liquid.thermal_conductivity,p.glass.thermal_conductivity);

        unsigned r_i = v.heater_rect.right / v.r.get_step();
        unsigned z_j = v.heater_rect.bottom / v.z.get_step();

        for (int i = 0; i < r_i; ++i) {
            T(i,z_j) = steel_to_water(T(i,z_j+1),T(i,z_j-1));
        }
        for (int j = 0; j < z_j; ++j) {
            T(r_i,j) = steel_to_water(T(r_i-1,j),T(r_i+1,j));
        }

        


        v.temperature_field.emplace_back(std::move(T));
        v.t+= dt;
    }
};
