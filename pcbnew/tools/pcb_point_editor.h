/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013-2017 CERN
 * Copyright (C) 2021-2023 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * @author Maciej Suminski <maciej.suminski@cern.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef PCB_POINT_EDITOR_H
#define PCB_POINT_EDITOR_H

#include <tool/tool_interactive.h>
#include "tool/edit_points.h"
#include <pcbnew_settings.h>
#include <status_popup.h>

#include <memory>


class PCB_SELECTION_TOOL;
class SHAPE_POLY_SET;

/**
 * Tool that displays edit points allowing to modify items by dragging the points.
 */
class PCB_POINT_EDITOR : public PCB_TOOL_BASE
{
public:
    PCB_POINT_EDITOR();

    /// @copydoc TOOL_INTERACTIVE::Reset()
    void Reset( RESET_REASON aReason ) override;

    /// @copydoc TOOL_INTERACTIVE::Init()
    bool Init() override;

    /**
     * Change selection event handler.
     */
    int OnSelectionChange( const TOOL_EVENT& aEvent );

    /**
     * Indicate the cursor is over an edit point.
     *
     * Used to coordinate cursor shapes with other tools.
     */
    bool HasPoint()    { return m_editedPoint != nullptr; }
    bool HasMidpoint() { return HasPoint() && dynamic_cast<EDIT_LINE*>( m_editedPoint ); }
    bool HasCorner()   { return HasPoint() && !HasMidpoint(); }

private:
    ///< Set up handlers for various events.
    void setTransitions() override;

    void buildForPolyOutline( std::shared_ptr<EDIT_POINTS> points, const SHAPE_POLY_SET* aOutline );

    std::shared_ptr<EDIT_POINTS> makePoints( EDA_ITEM* aItem );

    ///< Update item's points with edit points.
    void updateItem( BOARD_COMMIT* aCommit );

    /**
     * Validate a polygon and displays a popup warning if invalid.
     *
     * @param aModified is the polygon to be checked.
     * @return True if polygon is valid.
     */
    bool validatePolygon( SHAPE_POLY_SET& aModified ) const;

    ///< Update edit points with item's points.
    void updatePoints();

    ///< Update which point is being edited.
    void updateEditedPoint( const TOOL_EVENT& aEvent );

    ///< Set the current point being edited. NULL means none.
    void setEditedPoint( EDIT_POINT* aPoint );

    inline int getEditedPointIndex() const
    {
        for( unsigned i = 0; i < m_editPoints->PointsSize(); ++i )
        {
            if( m_editedPoint == &m_editPoints->Point( i ) )
                return i;
        }

        return wxNOT_FOUND;
    }

    ///< Return true if aPoint is the currently modified point.
    inline bool isModified( const EDIT_POINT& aPoint ) const
    {
        return m_editedPoint == &aPoint;
    }

    void pinEditedCorner( VECTOR2I& aTopLeft, VECTOR2I& aTopRight, VECTOR2I& aBotLeft,
                          VECTOR2I& aBotRight, const VECTOR2I& aHole = { 0, 0 },
                          const VECTOR2I& aHoleSize = { 0, 0 } ) const;

    ///< Set up an alternative constraint (typically enabled upon a modifier key being pressed).
    void setAltConstraint( bool aEnabled );

    ///< Return a point that should be used as a constrainer for 45 degrees mode.
    EDIT_POINT get45DegConstrainer() const;

    ///< Condition to display "Create corner" context menu entry.
    static bool addCornerCondition( const SELECTION& aSelection );

    ///< Determine if the tool can currently add a corner to the given item
    static bool canAddCorner( const EDA_ITEM& aItem );

    ///< Condition to display "Remove corner" context menu entry.
    bool removeCornerCondition( const SELECTION& aSelection );

    /// TOOL_ACTION handlers
    int movePoint( const TOOL_EVENT& aEvent );
    int addCorner( const TOOL_EVENT& aEvent );
    int removeCorner( const TOOL_EVENT& aEvent );
    int modifiedSelection( const TOOL_EVENT& aEvent );

    /**
     * Move an end point of the arc, while keeping the tangent at the other endpoint.
     */
    void editArcEndpointKeepTangent( PCB_SHAPE* aArc, const VECTOR2I& aCenter,
                                     const VECTOR2I& aStart, const VECTOR2I& aMid,
                                     const VECTOR2I& aEnd, const VECTOR2I& aCursor ) const;

    /**
     * Move an end point of the arc around the circumference.
     */
    void editArcEndpointKeepCenter( PCB_SHAPE* aArc, const VECTOR2I& aCenter,
                                    const VECTOR2I& aStart, const VECTOR2I& aMid,
                                    const VECTOR2I& aEnd, const VECTOR2I& aCursor ) const;

    /**
     * Move the arc center but keep endpoint locations.
     */
    void editArcCenterKeepEndpoints( PCB_SHAPE* aArc, const VECTOR2I& aCenter,
                                     const VECTOR2I& aStart, const VECTOR2I& aMid,
                                     const VECTOR2I& aEnd ) const;

    /**
     * Move the mid point of the arc, while keeping the two endpoints.
     */
    void editArcMidKeepEndpoints( PCB_SHAPE* aArc, const VECTOR2I& aStart, const VECTOR2I& aEnd,
                                  const VECTOR2I& aCursor ) const;

    /**
     * Move the mid point of the arc, while keeping the angle.
     */
    void editArcMidKeepCenter( PCB_SHAPE* aArc, const VECTOR2I& aCenter, const VECTOR2I& aStart,
                               const VECTOR2I& aMid, const VECTOR2I& aEnd,
                               const VECTOR2I& aCursor ) const;

    ///< Change the edit method for arcs.
    int changeArcEditMode( const TOOL_EVENT& aEvent );

private:
    PCB_SELECTION_TOOL*           m_selectionTool;
    std::shared_ptr<EDIT_POINTS>  m_editPoints;

    EDIT_POINT*                   m_editedPoint;
    EDIT_POINT*                   m_hoveredPoint;

    EDIT_POINT                    m_original;   ///< Original pos for the current drag point.

    ARC_EDIT_MODE                 m_arcEditMode;

    PCB_SELECTION                 m_preview;

    // Alternative constraint, enabled while a modifier key is held
    std::shared_ptr<EDIT_CONSTRAINT<EDIT_POINT>> m_altConstraint;
    EDIT_POINT                                   m_altConstrainer;

    bool                          m_inPointEditorTool; // Re-entrancy guard

    static const unsigned int COORDS_PADDING; // Padding from coordinates limits for this tool
};

#endif
