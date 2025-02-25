/**
 * @file risk_hybrid_a_star.h
 * @author Bolei Zhou
 * @brief This file is based on Fast-Planner (github.com/HKUST-Aerial-Robotics/Fast-Planner)
 * @version 1.0
 * @date 2022-09-29
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef _RISK_HYBRID_ASTAR_H
#define _RISK_HYBRID_ASTAR_H

#include <path_searching/dyn_a_star.h>
#include <path_searching/path_node.h>
// #include <plan_env/fake_dsp_map.h>
// #include <plan_env/risk_voxel.h>
#include <plan_env/risk_base.h>

#include <Eigen/Core>
#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>

class RiskHybridAstar : public AStar {
 private:
  /* ---------- search parameter ---------- */
  bool   is_testing_;
  bool   optimistic_;
  int    allocate_num_;  // number of nodes to be allocated in path_node_pool_
  int    check_num_;
  int    tolerance_;
  double max_tau_, init_max_tau_;
  double max_vel_, max_acc_;
  double w_time_, horizon_, lambda_heu_;
  double time_resolution_, inv_time_resolution_;
  double time_origin_;

  /* ---------- main data structure ---------- */
  /* Derived from Base Class */
  // int    rounds_{0};
  // double resolution_, inv_resolution_;
  // double tie_breaker_;
  // Eigen::Vector3i CENTER_IDX_, POOL_SIZE_;
  // Eigen::Vector3d map_center_;  // map center
  Eigen::Vector3d map_size_;

  RiskBase::Ptr grid_map_;
  // FakeRiskVoxel::Ptr grid_map_;
  // GridNodePtr ***          GridNodeMap_;
  std::vector<PathNodePtr>                                                   node_path_;
  NodeHashTable                                                              expanded_nodes_;
  std::vector<PathNodePtr>                                                   path_node_pool_;
  std::priority_queue<PathNodePtr, std::vector<PathNodePtr>, NodeComparator> open_set_;

  int use_node_num_, iter_num_;

  /* ---------- record data ---------- */
  bool                        is_shot_succ_ = false;
  bool                        has_path_     = false;
  double                      t_shot_;
  Eigen::Vector3d             start_vel_, end_vel_, start_acc_;
  Eigen::Matrix<double, 6, 6> phi_;  // state transit matrix
  Eigen::MatrixXd             coef_shot_;

  /* debug & visualization */
  std::vector<Eigen::Vector4d> occupied_voxels_;
  std::vector<Eigen::Vector4d> visited_voxels_;

  /* helper */
  int             timeToIndex(double time);
  Eigen::Vector3i posToIndex(Eigen::Vector3d pt);
  void            retrievePath(PathNodePtr end_node);

  /* shot trajectory */
  std::vector<double> cubic(double a, double b, double c, double d);
  std::vector<double> quartic(double a, double b, double c, double d, double e);
  bool   computeShotTraj(Eigen::VectorXd state1, Eigen::VectorXd state2, double time_to_goal);
  double estimateHeuristic(Eigen::VectorXd x1, Eigen::VectorXd x2, double& optimal_time);

  /* state propagation */
  void stateTransit(Eigen::Matrix<double, 6, 1>& state0,
                    Eigen::Matrix<double, 6, 1>& state1,
                    Eigen::Vector3d              um,
                    double                       tau);

 public:
  RiskHybridAstar() {}
  ~RiskHybridAstar();
  enum { REACH_HORIZON = 1, REACH_END = 2, NO_PATH = 3, NEAR_END = 4 };
  typedef std::shared_ptr<RiskHybridAstar> Ptr;

  /* main API */
  void setParam(ros::NodeHandle& nh);
  void setEnvironment(const RiskBase::Ptr& grid_map);
  void setMapCenter(const Eigen::Vector3d& map_center) { map_center_ = map_center; }
  void init(const Eigen::Vector3d& map_center, const Eigen::Vector3d& map_size);
  void reset();

  ASTAR_RET search(Eigen::Vector3d start_pt,
                   Eigen::Vector3d start_vel,
                   Eigen::Vector3d start_acc,
                   Eigen::Vector3d end_pt,
                   Eigen::Vector3d end_vel,
                   bool            init,
                   bool            dynamic    = false,
                   double          time_start = -1.0);

  std::vector<Eigen::Vector3d>             getPath(double delta_t);
  std::vector<Eigen::Matrix<double, 6, 1>> getPathWithVel(double delta_t);
  std::vector<Eigen::Vector4d>             getTraversedObstacles() { return visited_voxels_; }
  std::vector<Eigen::Vector4d>             getOccupiedObstacles() { return occupied_voxels_; }

  void getSamples(double&                       ts,
                  std::vector<Eigen::Vector3d>& point_set,
                  std::vector<Eigen::Vector3d>& start_end_derivatives);

  std::vector<PathNodePtr> getVisitedNodes();

 private:
  inline int checkOccupancy(const Eigen::Vector3d& pos);
};

/* ----- inline function -----*/
inline int RiskHybridAstar::checkOccupancy(const Eigen::Vector3d& pos) {
  return grid_map_->getInflateOccupancy(pos);
}

#endif  // _KINODYNAMIC_ASTAR_H
