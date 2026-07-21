#include "videoplayer_window.h"
#include "ui_player.h"

#include <QFileDialog>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QVideoWidget>
#include <QVBoxLayout>

Q_LOGGING_CATEGORY(lcPlayer, "app.player")

// ── Constructor / Destructor ───────────────────────────────────────────────

VideoPlayerWindow::VideoPlayerWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui_(std::make_unique<Ui::MainWindow>())
    , player_(new QMediaPlayer(this))          // NOLINT(cppcoreguidelines-owning-memory) — Qt-parented
    , audioOutput_(new QAudioOutput(this))     // NOLINT(cppcoreguidelines-owning-memory) — Qt-parented
{
    ui_->setupUi(this);
    setWindowTitle(tr("Qt6 Video Player"));

    setupPlayer();
    setupConnections();
    updateButtonStates(QMediaPlayer::StoppedState);

    qCInfo(lcPlayer) << "VideoPlayerWindow initialized";
}

VideoPlayerWindow::~VideoPlayerWindow() = default;

// ── Public API ─────────────────────────────────────────────────────────────

bool VideoPlayerWindow::loadFile(const QString& filePath)
{
    if (filePath.isEmpty()) {
        qCWarning(lcPlayer) << "loadFile: empty path";
        return false;
    }

    player_->stop();
    player_->setSource(QUrl::fromLocalFile(filePath));
    qCInfo(lcPlayer) << "Loading file:" << filePath;

    // QMediaPlayer reports errors asynchronously via errorOccurred()
    return true;
}

// ── Private setup ──────────────────────────────────────────────────────────

void VideoPlayerWindow::setupPlayer()
{
    player_->setAudioOutput(audioOutput_);
    audioOutput_->setVolume(0.8f);

    // Replace the QGraphicsView placeholder in the .ui with a QVideoWidget
    auto* videoWidget = new QVideoWidget(this);  // NOLINT(cppcoreguidelines-owning-memory) — Qt-parented
    player_->setVideoOutput(videoWidget);

    // Swap placeholder with the actual video widget
    auto* layout = qobject_cast<QVBoxLayout*>(
        ui_->verticalLayoutWidget->layout());
    if (layout) {
        // Replace index 0 (the QGraphicsView) with the video widget.
        // takeAt() transfers ownership of the QLayoutItem (and its widget)
        // to the caller — both must be deleted explicitly per Qt's contract.
        QLayoutItem* old = layout->takeAt(0);
        delete old->widget();  // NOLINT(cppcoreguidelines-owning-memory)
        delete old;            // NOLINT(cppcoreguidelines-owning-memory)
        layout->insertWidget(0, videoWidget);
    }
}

void VideoPlayerWindow::setupConnections()
{
    // Toolbar buttons
    connect(ui_->playButton,    &QPushButton::clicked, this, &VideoPlayerWindow::onPlayClicked);
    connect(ui_->pauseButton,   &QPushButton::clicked, this, &VideoPlayerWindow::onPauseClicked);
    connect(ui_->StopButton,    &QPushButton::clicked, this, &VideoPlayerWindow::onStopClicked);
    connect(ui_->rewindButton,  &QPushButton::clicked, this, &VideoPlayerWindow::onRewindClicked);
    connect(ui_->forwardButton, &QPushButton::clicked, this, &VideoPlayerWindow::onForwardClicked);

    // Player state
    connect(player_, &QMediaPlayer::playbackStateChanged,
            this,    &VideoPlayerWindow::onPlayerStateChanged);
    connect(player_, &QMediaPlayer::errorOccurred,
            this,    &VideoPlayerWindow::onPlayerError);
    connect(player_, &QMediaPlayer::durationChanged,
            this,    &VideoPlayerWindow::onDurationChanged);
    connect(player_, &QMediaPlayer::positionChanged,
            this,    &VideoPlayerWindow::onPositionChanged);

    // Menu → File → Open
    // (Added programmatically since the .ui has no menu items)
    auto* fileMenu   = menuBar()->addMenu(tr("&File"));
    auto* openAction = fileMenu->addAction(tr("&Open…"), QKeySequence::Open);
    connect(openAction, &QAction::triggered,
            this,       &VideoPlayerWindow::onOpenFileTriggered);

    auto* quitAction = fileMenu->addAction(tr("&Quit"), QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);
}

// ── Slots ──────────────────────────────────────────────────────────────────

void VideoPlayerWindow::onOpenFileTriggered()
{
    const QString path = QFileDialog::getOpenFileName(
        this,
        tr("Open Video"), {},
        tr("Video Files (*.mp4 *.avi *.mkv *.mov *.wmv *.flv);;"
           "All Files (*)"));
    if (!path.isEmpty() && !loadFile(path)) {
        QMessageBox::warning(this, tr("Open Video"),
                              tr("Could not load the selected file."));
    }
}

void VideoPlayerWindow::onPlayClicked()
{
    if (player_->source().isEmpty()) {
        onOpenFileTriggered();
        return;
    }
    player_->play();
    qCDebug(lcPlayer) << "Play";
}

void VideoPlayerWindow::onPauseClicked()
{
    player_->pause();
    qCDebug(lcPlayer) << "Pause";
}

void VideoPlayerWindow::onStopClicked()
{
    player_->stop();
    qCDebug(lcPlayer) << "Stop";
}

void VideoPlayerWindow::onRewindClicked()
{
    const qint64 pos = qMax(0LL, player_->position() - kSeekStepMs);
    player_->setPosition(pos);
    qCDebug(lcPlayer) << "Rewind to" << pos << "ms";
}

void VideoPlayerWindow::onForwardClicked()
{
    const qint64 pos = qMin(player_->duration(),
                             player_->position() + kSeekStepMs);
    player_->setPosition(pos);
    qCDebug(lcPlayer) << "Forward to" << pos << "ms";
}

void VideoPlayerWindow::onPlayerStateChanged(QMediaPlayer::PlaybackState state)
{
    qCInfo(lcPlayer) << "State changed:" << state;
    updateButtonStates(state);
}

void VideoPlayerWindow::onPlayerError(QMediaPlayer::Error /*error*/,
                                       const QString& errorString)
{
    qCWarning(lcPlayer) << "Player error:" << errorString;
    QMessageBox::warning(this, tr("Playback Error"), errorString);
    updateButtonStates(QMediaPlayer::StoppedState);
}

void VideoPlayerWindow::onDurationChanged(qint64 duration)
{
    ui_->progressBar->setMaximum(static_cast<int>(duration / kMsPerSecond));
}

void VideoPlayerWindow::onPositionChanged(qint64 position)
{
    ui_->progressBar->setValue(static_cast<int>(position / kMsPerSecond));
}

// ── Helpers ────────────────────────────────────────────────────────────────

void VideoPlayerWindow::updateButtonStates(QMediaPlayer::PlaybackState state)
{
    const bool playing = (state == QMediaPlayer::PlayingState);
    const bool stopped = (state == QMediaPlayer::StoppedState);

    ui_->playButton->setEnabled(!playing);
    ui_->pauseButton->setEnabled(playing);
    ui_->StopButton->setEnabled(!stopped);
    ui_->rewindButton->setEnabled(!stopped);
    ui_->forwardButton->setEnabled(!stopped);

    statusBar()->showMessage(
        playing ? tr("Playing") :
        (state == QMediaPlayer::PausedState) ? tr("Paused") : tr("Stopped"));
}
