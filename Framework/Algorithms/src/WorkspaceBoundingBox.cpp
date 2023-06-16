#include "MantidAlgorithms/WorkspaceBoundingBox.h"
#include "MantidAPI/MatrixWorkspace.h"

namespace Mantid::Algorithms {

namespace {
constexpr int HISTOGRAM_INDEX{0};
}

WorkspaceBoundingBox::WorkspaceBoundingBox(const API::MatrixWorkspace_const_sptr &workspace, const double beamRadius,
                                           const bool ignoreDirectBeam)
    : m_workspace(workspace), m_beamRadiusSq(beamRadius * beamRadius), m_ignoreDirectBeam(ignoreDirectBeam) {
  if (m_workspace->y(0).size() != 1)
    throw std::runtime_error("This object only works with integrated workspaces");

  m_spectrumInfo = &workspace->spectrumInfo();
  m_numSpectra = m_workspace->getNumberHistograms();
}

WorkspaceBoundingBox::WorkspaceBoundingBox() {
  m_spectrumInfo = nullptr; // certain functionality is not available
}

WorkspaceBoundingBox::~WorkspaceBoundingBox() = default;

Kernel::V3D WorkspaceBoundingBox::position(const std::size_t index) const {
  if (!m_spectrumInfo)
    throw std::runtime_error("SpectrumInfo object is not initialized");

  return m_spectrumInfo->position(index);
}

// find the min/max for x/y coords in set of valid spectra, update position of bounding box
void WorkspaceBoundingBox::initBoundingBox() {
  double totalCount = 0;
  for (std::size_t i = 0; i < m_numSpectra; i++) {
    if (!isValidIndex(i))
      continue;

    updateMinMax(i);

    if (includeInIntegration(i))
      totalCount += updatePositionAndReturnCount(i);
  }
  this->normalizePosition(totalCount, totalCount);
}

// In subsequent iterations check if spectra fits in the normalized bounding box(generated by previous iterations)
// If so, update position
void WorkspaceBoundingBox::updateBoundingBox(WorkspaceBoundingBox &previousBoundingBox) {
  double totalCount = 0;
  const auto &spectrumInfo = getWorkspace()->spectrumInfo();
  for (std::size_t i = 0; i < m_numSpectra; i++) {
    if (!isValidIndex(i))
      continue;

    const V3D position = spectrumInfo.position(i);

    if (previousBoundingBox.containsPoint(position.X(), position.Y()) && includeInIntegration(position)) {
      totalCount += updatePositionAndReturnCount(i);
    }
  }
  this->normalizePosition(totalCount, totalCount);
}

double WorkspaceBoundingBox::countsValue(const std::size_t index) const {
  return m_workspace->y(index)[HISTOGRAM_INDEX];
}

void WorkspaceBoundingBox::setPosition(const double x, const double y) {
  this->m_xPos = x;
  this->m_yPos = y;
}

void WorkspaceBoundingBox::setCenter(const double x, const double y) {
  this->m_centerXPos = x;
  this->m_centerYPos = y;
}

void WorkspaceBoundingBox::setBounds(const double xMin, const double xMax, const double yMin, const double yMax) {
  this->m_xPosMin = xMin;
  this->m_xPosMax = xMax;
  this->m_yPosMin = yMin;
  this->m_yPosMax = yMax;
}

/** Performs checks on the spectrum located at index to determine if
 *  it is acceptable to be operated on
 *
 *  @param index :: index of spectrum data
 *  @return true/false if its valid
 */
bool WorkspaceBoundingBox::isValidIndex(const std::size_t index) const {
  if (!m_spectrumInfo)
    throw std::runtime_error("SpectrumInfo object is not initialized");
  if (!m_spectrumInfo->hasDetectors(index)) {
    g_log.warning() << "Workspace index " << index << " has no detector assigned to it - discarding\n";
    return false;
  }
  // Skip if we have a monitor or if the detector is masked.
  if (this->m_spectrumInfo->isMonitor(index) || this->m_spectrumInfo->isMasked(index))
    return false;

  // Get the current spectrum
  const auto YIn = this->countsValue(index);
  // Skip if NaN of inf
  if (std::isnan(YIn) || std::isinf(YIn))
    return false;
  return true;
}

/** Searches for the first valid spectrum info in member variable `workspace`
 *
 *  @param numSpec :: the number of spectrum in the workspace to search through
 *  @return index of first valid spectrum
 */
std::size_t WorkspaceBoundingBox::findFirstValidWs(const std::size_t numSpec) const {
  std::size_t i;
  for (i = 0; i < numSpec; ++i) {
    if (isValidIndex(i))
      break;
  }
  return i;
}

/** Sets member variables x/y to new x/y based on
 *  spectrum info and historgram data at the given index
 *
 *  @param index :: index of spectrum data
 *  @return number of points of histogram data at index
 */
double WorkspaceBoundingBox::updatePositionAndReturnCount(const std::size_t index) {
  const auto counts = this->countsValue(index);
  const auto &position = this->position(index);

  this->m_xPos += counts * position.X();
  this->m_yPos += counts * position.Y();

  return counts;
}

/** Compare current mins and maxs to the coordinates of the spectrum at index
 *  expnd mins and maxs to include this spectrum
 *
 *  @param index :: index of spectrum data
 */
void WorkspaceBoundingBox::updateMinMax(const std::size_t index) {
  const auto &position = this->position(index);
  const double x = position.X();
  const double y = position.Y();

  this->m_xPosMin = std::min(x, this->m_xPosMin);
  this->m_xPosMax = std::max(x, this->m_xPosMax);
  this->m_yPosMin = std::min(y, this->m_yPosMin);
  this->m_yPosMax = std::max(y, this->m_yPosMax);
}

/** Checks to see if spectrum at index should be included in the integration
 *
 *  @param index :: index of spectrum data
 */
bool WorkspaceBoundingBox::includeInIntegration(const std::size_t index) {
  return this->includeInIntegration(this->position(index));
}

bool WorkspaceBoundingBox::includeInIntegration(const Kernel::V3D &position) {
  if (m_ignoreDirectBeam) {
    const double dx = position.X() - this->m_centerXPos;
    const double dy = position.Y() - this->m_centerYPos;

    return bool(dx * dx + dy * dy >= m_beamRadiusSq);
  }
  return true;
}

double WorkspaceBoundingBox::calculateDistance() const {
  const auto xExtent = (m_centerXPos - m_xPos);
  const auto yExtent = (m_centerYPos - m_yPos);
  return sqrt(xExtent * xExtent + yExtent * yExtent);
}

double WorkspaceBoundingBox::calculateRadiusX() const { return std::min((m_xPos - m_xPosMin), (m_xPosMax - m_xPos)); }

double WorkspaceBoundingBox::calculateRadiusY() const { return std::min((m_yPos - m_yPosMin), (m_yPosMax - m_yPos)); }

/** Perform normalization on x/y coords over given values
 *
 *  @param x :: value to normalize member x over
 *  @param y :: value to normalize member y over
 */
void WorkspaceBoundingBox::normalizePosition(double x, double y) {
  this->m_xPos /= std::fabs(x);
  this->m_yPos /= std::fabs(y);
}

/** Checks if a given x/y coord is within the bounding box
 *
 *  @param x :: x coordinate
 *  @param y :: y coordinate
 *  @return true/false if it is within the mins/maxs of the box
 */
bool WorkspaceBoundingBox::containsPoint(double x, double y) {
  return (x <= this->m_xPosMax && x >= this->m_xPosMin && y <= m_yPosMax && y >= m_yPosMin);
}

} // namespace Mantid::Algorithms
