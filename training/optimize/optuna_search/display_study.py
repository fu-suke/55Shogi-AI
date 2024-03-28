import joblib
import optuna

study = joblib.load('./studies/study_20240105200522.pkl')

best_trial = study.best_trial
print(f"Best Trial Number: {best_trial.number}")
print(f"Best Trial Value (Objective): {best_trial.value}")

print("Best Trial Parameters:")
for key, value in best_trial.params.items():
    print(f"  {key}: {value}")

print("\nStudy Statistics:")
print(f"  Number of Finished Trials: {len(study.trials)}")
print(
    f"  Number of Pruned Trials: {sum(trial.state == optuna.trial.TrialState.PRUNED for trial in study.trials)}")
print(
    f"  Number of Complete Trials: {sum(trial.state == optuna.trial.TrialState.COMPLETE for trial in study.trials)}")

# 完了したトライアルのみを抽出
completed_trials = [
    trial for trial in study.trials if trial.state == optuna.trial.TrialState.COMPLETE]

# 完了したトライアルの情報を表示
print("Completed Trials:")
for trial in completed_trials:
    print(f"Trial Number: {trial.number}")
    print(f"  Value (Objective): {trial.value}")
    print("  Parameters:")
    for key, value in trial.params.items():
        print(f"    {key}: {value}")
    print()


# ハイパーパラメータの重要度を取得
param_importances = optuna.importance.get_param_importances(study)

# ハイパーパラメータの重要度を表示
print("Parameter Importances:")
for param, importance in param_importances.items():
    print(f"  {param}: {importance}")
