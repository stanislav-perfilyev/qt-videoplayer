#pragma once

#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <memory>

Q_DECLARE_LOGGING_CATEGORY(lcPlayer)

namespace Ui { class MainWindow; }

/**
 * @class VideoPlayerWindow
 * @brief Main window for the Qt6 video player.
 *
 * Wraps QMediaPlayer + QVideoWidget and exposes a clean API for
 * loading and controlling video playback.
 *
 * Controls available via UI:
 *   Play / Pause / Stop / Rewind (−10 s) / Forward (+10 s)
 *   Progress bar (seek on click not implemented yet — see TODO)
 *
 * Non-copyable and non-movable (Qt widget ownership semantics).
 */
class VideoPlayerWindow : public QMainWindow {
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(VideoPlayerWindow)

public:
    /**
     * @brief Construct the player window.
     * @param parent Parent widget (default: nullptr).
     */
    explicit VideoPlayerWindow(QWidget *parent = nullptr);
    ~VideoPlayerWindow() override;

    /**
     * @brief Open a video file and prepare it for playback.
     * @param filePath Absolute or relative path to the video file.
     * @return true  if the file was accepted by the media backend;
     *         false if the path is empty or the format is unsupported.
     */
    [[nodiscard]] bool loadFile(const QString& filePath);

private slots:
    void onOpenFileTriggered();

    void onPlayClicked();
    void onPauseClicked();
    void onStopClicked();
    void onRewindClicked();
    void onForwardClicked();

    void onPlayerStateChanged(QMediaPlayer::PlaybackState state);
    void onPlayerError(QMediaPlayer::Error error, const QString& errorString);
    void onDurationChanged(qint64 duration);
    void onPositionChanged(qint64 position);

private:
    void setupPlayer();
    void setupConnections();
    void updateButtonStates(QMediaPlayer::PlaybackState state);

    static constexpr qint64 kSeekStepMs = 10'000;  ///< 10 seconds in ms

    std::unique_ptr<Ui::MainWindow> ui_;
    QMediaPlayer*                   player_;      ///< owned by Qt parent chain
    QAudioOutput*                   audioOutput_; ///< owned by Qt parent chain
};
